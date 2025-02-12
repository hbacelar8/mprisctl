# MPRIS Controller Library

C++ MPRIS controller library implementation, inspired on the functionnalities of
the [playerctl](https://github.com/altdesktop/playerctl) project.

This is a library though, not intented to be used as a CLI tool.

## Configure and build

In the project's root:

```bash
cmake --preset example
```

```bash
cmake --build build
```

## Run the example

```bash
./build/examples/example
```

## Example usage

```cpp
#include <iostream>
#include "mprisctl.h"

int main()
{
  // Instantiate the MPRIS controller, opening the ubs connection
  mpris_ctl::MPRISController mpris_controller;

  // Get a list of all players
  auto players = mpris_controller.GetAvailablePlayers();

  // Print all players
  std::cout << "List of all players:" << std::endl;
  for (auto player : players)
  {
    std::cout << "- " << player << std::endl;
  }

  // Get first player
  std::string player = players[0];

  // Toogle play/pause
  mpris_controller.PlayPause(player);

  // Get metadata and print it
  std::cout << "\nMetadata:" << std::endl;
  mpris_ctl::TrackMetadata metatada = mpris_controller.GetMetadata(player);
  std::cout << "Title: " << metatada.title << std::endl;
  std::cout << "Artist: " << metatada.artist << std::endl;
  std::cout << "Album: " << metatada.album << std::endl;
  std::cout << "Art URL: " << metatada.artUrl << std::endl;
  std::cout << "URL: " << metatada.url << std::endl;
  std::cout << "Length: " << metatada.length << std::endl;

  // Toggle play/pause
  mpris_controller.PlayPause(player);

  return 0;
}
```