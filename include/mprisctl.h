#ifndef _MPRISCTL_H_
#define _MPRISCTL_H_

#include <functional>
#include <string>

// Forward declarations
struct DBusConnection;
struct DBusMessageIter;

namespace mpris_ctl
{

/**
 * @class TrackMetadata
 * @brief Track metadata structure
 *
 */
struct TrackMetadata
{
  std::string title;
  std::string artist;
  std::string album;
  std::string artUrl;
  std::string url;
  int64_t length;
};

/**
 * @class MPRISController
 * @brief MPRIS controller class
 *
 */
class MPRISController
{
public:
  /**
   * @brief Create a new MPRISController object
   *
   * @note Creating a new MPRISController object will open a D-BUS connection
   */
  MPRISController();

  /**
   * @brief Destroy the MPRISController object
   */
  ~MPRISController();

  /**
   * @brief Get a vector with all available players
   *
   * @return std::vector<std::string> Vector of available players
   * @note An empty vector is returned if failed
   */
  std::vector<std::string> GetAvailablePlayers();

  /**
   * @brief Toggle Play/Pause on the specified player
   *
   * @param playerName Name of the player
   * @return Whether or not the command was successfully done
   */
  bool PlayPause(const std::string& playerName);

  /**
   * @brief Play next media on the specified player
   *
   * @param playerName Name of the player
   * @return Whether or not the command was successfully done
   */
  bool Next(const std::string& playerName);

  /**
   * @brief Play the previous media on the specified player
   *
   * @param playerName Name of the player
   * @return Whether or not the command was successfully done
   */
  bool Previous(const std::string& playerName);

  /**
   * @brief Get track metadata for the specified player
   *
   * @param playerName Name of the player
   * @return Track metadata
   */
  TrackMetadata GetMetadata(const std::string& playerName);

private:
  /**
   * @brief Send a command to the MPRIS bus
   *
   * @param playerName Name of the player on the bus
   * @param method Function to apply to the bus
   * @return Whether or not the command was successfully done
   */
  bool SendCommand(const std::string& playerName, const std::string& method);

  /**
   * @brief Metadata handlers creator
   *
   * @return Unordered map of metadata handlers
   */
  std::unordered_map<std::string, std::function<void(DBusMessageIter&, TrackMetadata&)>> CreateMetadataHandlers();

  /**
   * @brief D-Bus connection
   */
  DBusConnection* dbus_connection {nullptr};
};

}  // namespace mpris_ctl

#endif  // !_MPRISCTL_H_
