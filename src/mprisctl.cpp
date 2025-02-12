#include "mprisctl.h"

#include <dbus/dbus-protocol.h>
#include <dbus/dbus.h>
#include <string.h>
#include <functional>
#include <iostream>

// Defines
#define MPRIS_PATH               "/org/mpris/MediaPlayer2"
#define MPRIS_PLAYER_INTERFACE   "org.mpris.MediaPlayer2.Player"
#define MPRIS_METADATA_INTERFACE "org.freedesktop.DBus.Properties"

namespace mpris_ctl
{

MPRISController::MPRISController()
{
  DBusError error;
  dbus_error_init(&error);

  // Connect to the bus
  dbus_connection = dbus_bus_get(DBUS_BUS_SESSION, &error);

  if (dbus_error_is_set(&error))
  {
    std::cerr << "Failed to connect to the session bus: " << error.message << std::endl;

    // Free error from memory
    dbus_error_free(&error);

    // Unreference bus
    dbus_connection = nullptr;
  }
}

MPRISController::~MPRISController()
{
  if (dbus_connection)
    dbus_connection_unref(dbus_connection);
}

std::vector<std::string> MPRISController::GetAvailablePlayers()
{
  std::vector<std::string> players;

  if (!dbus_connection)
  {
    std::cerr << "D-Bus connection is not open" << std::endl;

    return players;
  }

  DBusError error;
  dbus_error_init(&error);

  // Create a new method call message
  DBusMessage *msg = dbus_message_new_method_call("org.freedesktop.DBus",
                                                  "/org/freedesktop/DBus",
                                                  "org.freedesktop.DBus",
                                                  "ListNames");

  if (!msg)
  {
    std::cerr << "Failed to create D-Bus message" << std::endl;

    return players;
  }

  // Send message and wait for a reply
  DBusMessage *reply = dbus_connection_send_with_reply_and_block(dbus_connection, msg, -1, &error);

  if (dbus_error_is_set(&error))
  {
    std::cerr << "Failed to get available players: " << error.message << std::endl;

    // Clean up
    dbus_error_free(&error);
  }
  else
  {
    DBusMessageIter iter;

    if (dbus_message_iter_init(reply, &iter))
    {
      DBusMessageIter arrayIter;
      dbus_message_iter_recurse(&iter, &arrayIter);

      while (dbus_message_iter_get_arg_type(&arrayIter) != DBUS_TYPE_INVALID)
      {
        const char *serviceName;
        dbus_message_iter_get_basic(&arrayIter, &serviceName);

        // Check if the service name matches the MPRIS naming convention

        if (std::string(serviceName).find("org.mpris.MediaPlayer2.") == 0)
        {
          std::string playerName = std::string(serviceName);
          playerName.erase(0, strlen("org.mpris.MediaPlayer2."));
          players.push_back(playerName);
        }

        dbus_message_iter_next(&arrayIter);
      }
    }
    else
    {
      std::cerr << "Failed to parse metada message" << std::endl;
    }
  }

  // Clean up
  if (msg)
    dbus_message_unref(msg);

  if (reply)
    dbus_message_unref(reply);

  return players;
}

bool MPRISController::PlayPause(const std::string &playerName)
{
  return SendCommand(playerName, "PlayPause");
}

bool MPRISController::Next(const std::string &playerName)
{
  return SendCommand(playerName, "Next");
}

bool MPRISController::Previous(const std::string &playerName)
{
  return SendCommand(playerName, "Previous");
}

TrackMetadata MPRISController::GetMetadata(const std::string &playerName)
{
  static const auto handlers = MPRISController::CreateMetadataHandlers();
  TrackMetadata metadata     = {};

  if (!dbus_connection)
  {
    std::cerr << "D-Bus connection is not open" << std::endl;

    return metadata;
  }

  // Create a new method call message to get the metadata property
  DBusMessage *msg = dbus_message_new_method_call(("org.mpris.MediaPlayer2." + playerName).c_str(),
                                                  MPRIS_PATH,
                                                  MPRIS_METADATA_INTERFACE,
                                                  "Get");

  if (!msg)
  {
    std::cerr << "Failed to create D-Bus message" << std::endl;

    return metadata;
  }

  // Append arguments to the method call
  const char *interface = "org.mpris.MediaPlayer2.Player";
  const char *property  = "Metadata";
  dbus_message_append_args(msg, DBUS_TYPE_STRING, &interface, DBUS_TYPE_STRING, &property, DBUS_TYPE_INVALID);

  DBusError error;
  dbus_error_init(&error);

  // Send message and wait for a reply
  DBusMessage *reply = dbus_connection_send_with_reply_and_block(dbus_connection, msg, -1, &error);

  if (dbus_error_is_set(&error))
  {
    std::cerr << "Failed to get track metadata: " << error.message << std::endl;
  }
  else
  {
    // Parse the reply to extract metadata
    DBusMessageIter iter;

    if (dbus_message_iter_init(reply, &iter))
    {
      DBusMessageIter variantIter, dictIter;

      // Iterate until metadata dictionary
      dbus_message_iter_recurse(&iter, &variantIter);
      dbus_message_iter_recurse(&variantIter, &dictIter);

      // Iterate over dictionary object
      while (dbus_message_iter_get_arg_type(&dictIter) != DBUS_TYPE_INVALID)
      {
        DBusMessageIter entryIter;
        dbus_message_iter_recurse(&dictIter, &entryIter);

        // Get the key
        const char *key;
        dbus_message_iter_get_basic(&entryIter, &key);
        dbus_message_iter_next(&entryIter);

        // Get the value
        DBusMessageIter valueIter;
        dbus_message_iter_recurse(&entryIter, &valueIter);

        // Handle the value with the lookup table
        auto handler = handlers.find(key);
        if (handler != handlers.end())
        {
          // Call the handler
          handler->second(valueIter, metadata);
        }

        // Iterate to next dictionary entry
        dbus_message_iter_next(&dictIter);
      }
    }
    else
    {
      std::cerr << "Failed to parse metada message" << std::endl;
    }
  }

  // Clean up
  dbus_error_free(&error);
  if (msg)
    dbus_message_unref(msg);
  if (reply)
    dbus_message_unref(reply);

  return metadata;
}

bool MPRISController::SendCommand(const std::string &playerName, const std::string &method)
{
  bool success(true);

  if (!dbus_connection)
  {
    std::cerr << "D-Bus connection is not open" << std::endl;

    success = false;
  }
  else
  {
    DBusError error;
    dbus_error_init(&error);

    // Create a new method call message
    DBusMessage *msg = dbus_message_new_method_call(("org.mpris.MediaPlayer2." + playerName).c_str(),
                                                    MPRIS_PATH,
                                                    MPRIS_PLAYER_INTERFACE,
                                                    method.c_str());

    if (!msg)
    {
      std::cerr << "Failed to create D-Bus message" << std::endl;

      success = false;
    }
    else
    {
      // Send message and wait for a reply
      DBusMessage *reply = dbus_connection_send_with_reply_and_block(dbus_connection, msg, -1, &error);

      if (dbus_error_is_set(&error))
      {
        std::cerr << "Failed to send " << method << " command: " << error.message << std::endl;

        // Clean up
        dbus_error_free(&error);

        success = false;
      }

      // Clean up
      if (msg)
        dbus_message_unref(msg);

      if (reply)
        dbus_message_unref(reply);
    }
  }

  return success;
}

std::unordered_map<std::string, std::function<void(DBusMessageIter &, TrackMetadata &)>>
    MPRISController::CreateMetadataHandlers()
{
  return {{"xesam:title",
           [](DBusMessageIter &iter, TrackMetadata &metadata)
           {
             const char *value;
             dbus_message_iter_get_basic(&iter, &value);
             metadata.title = value;
           }},
          {"xesam:album",
           [](DBusMessageIter &iter, TrackMetadata &metadata)
           {
             const char *value;
             dbus_message_iter_get_basic(&iter, &value);
             metadata.album = value;
           }},
          {"xesam:artist",
           [](DBusMessageIter &iter, TrackMetadata &metadata)
           {
             DBusMessageIter arrayIter;
             std::string arrayValue;
             dbus_message_iter_recurse(&iter, &arrayIter);

             while (dbus_message_iter_get_arg_type(&arrayIter) != DBUS_TYPE_INVALID)
             {
               if (dbus_message_iter_get_arg_type(&arrayIter) == DBUS_TYPE_STRING)
               {
                 const char *arrayItem;
                 dbus_message_iter_get_basic(&arrayIter, &arrayItem);

                 if (!arrayValue.empty())
                 {
                   arrayValue += ", ";
                 }
                 arrayValue += arrayItem;
               }
               dbus_message_iter_next(&arrayIter);
             }

             metadata.artist = arrayValue;
           }},
          {"mpris:artUrl",
           [](DBusMessageIter &iter, TrackMetadata &metadata)
           {
             const char *value;
             dbus_message_iter_get_basic(&iter, &value);
             metadata.artUrl = value;
           }},
          {"xesam:url",
           [](DBusMessageIter &iter, TrackMetadata &metadata)
           {
             const char *value;
             dbus_message_iter_get_basic(&iter, &value);
             metadata.url = value;
           }},
          {"mpris:length",
           [](DBusMessageIter &iter, TrackMetadata &metadata)
           {
             dbus_int64_t length;
             dbus_message_iter_get_basic(&iter, &length);
             metadata.length = length;
           }}};
}

}  // namespace mpris_ctl
