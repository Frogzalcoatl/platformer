# Platformer
A simple platformer project to keep my C++ knowledge sharp while taking data structures. Improves upon my previous Connect4 SDL project. Build process uses Vcpkg, Cmake, and Ninja. Includes the following libraries:

- [SDL3](https://github.com/libsdl-org/SDL)
- [SDL3 Image](https://github.com/libsdl-org/SDL_image)
- [SDL3 TTF](https://github.com/libsdl-org/SDL_ttf)
- [SDL3 Mixer](https://github.com/libsdl-org/SDL_mixer)
- [Box2D](https://github.com/erincatto/box2d)
- [ImGui](https://github.com/ocornut/imgui)
- [Rapid JSON](https://github.com/Tencent/rapidjson)
- [Discord RPC](https://github.com/discord/discord-rpc)

### Disclaimer:
I am by no means a professonal at what I do yet and occasionally use AI as a learning tool. Any decisions or implementations influenced by AI are clearly marked through comments. If AI has led me astray in any of these places, please feel free to let me know. Also since I'm still learning, if you have suggestions anywhere else, please let me know about those too. Thanks!

## Features
* Compatible with Windows, MacOS, Linux, and Android.
* Uses SDL3 for video, audio, input, and file management.
* Local multiplayer with keyboard, touch, and controller support.
* Uses Box2D to handle physics.
* Uses Rapid JSON to store user data like settings between reloads.
* Uses Dear ImGui to manage basic gui.
* Displays discord status using Discord RPC.

## Project Scope
I originally wanted this platformer game to include an 8 world story mode similar to "New Super Mario Bros", a level editor, and online multiplayer. While I may revisit this idea in the future, I would like to move on to other projects and learn skills outside of tedious cpp game development.

This project will include a less than 10 level story mode, compatible with local multiplayer. That will be it.