# Introduction
This program enables you to solve simple sliding puzzle games. A descrition of the game type is given on this [wikipedia article](https://en.wikipedia.org/wiki/Sliding_puzzle).

<div align="center">
  <a href="https://www.mad-weasel.de/sliding.html"
     target="_blank">
    <img src="https://www.mad-weasel.de/images/sliding_gui_play.png"
         alt="Ingame screenshot"
         width="240" height="180" border="10" />
  </a>
  <a href="https://www.mad-weasel.de/sliding.html"
     target="_blank">
    <img src="https://www.mad-weasel.de/images/sliding_gui_setting_state.png"
         alt="Ingame screenshot"
         width="240" height="180" border="10" />
  </a>  
</div>

The principle of the solving algorithmn is explained in <a href="https://www.mad-weasel.de/download/Solving the Sliding Puzzle Game.pdf" target="_blank">this</a> pdf document.

The current executable can be downloaded [here](https://www.mad-weasel.de/sliding.html). Just download the corresponding zip file and extract the files in a folder.

# Software dependencies
## Libraries
This application uses the following EXTERNAL libraries: 
- [DirectX 11 Toolkit](https://github.com/Microsoft/DirectXTK)
- shlwapi.lib, comctl32.lib, DXGI.lib, D3D11.lib, XmlLite.lib
- Win32 API
- C++ STL
## Tools
The following EXTERNAL tools were used for the source code and executable generation: 
- MS Visual Studio Community 2017
- [ResEdit](http://www.resedit.net/)
- GitHub Extension for Visual Studio
## Environment
System requirements:
- Windows 10
- DirectX 11 (since the DirectXTK 11 is used)

# Latest releases
January 1, 2019 - First release.

# Build
- Install MS Visual Studio Community 2017
  - NuGet-Paket-Manager (for DirectXTK 11)
  - VC++ 2017
  - Visual Studio C++-Corefeatures
  - Windows 10 SDK (> 10.0.17763.0)
- Install GitHub Extension for Visual Studio (if no other github tool is already installed)
- Clone the [DirectXTK 11 git repository](https://github.com/Microsoft/DirectXTK)
- Clone this repositority
- Clone the library repository [weaselLibrary](https://github.com/madweasel/madweasels-cpp)
- Open the project via the .sln file (Traga2Win.sln)
- Compile and run

# Contribute
TODO: Explain how other users and developers can contribute to make your code better. 

Contact: [karaizy@mad-weasel.de](mailto:karaizy@mad-weasel.de).

# License
Copyright (c) Thomas Weber. All rights reserved.

Licensed under the [MIT](LICENSE.txt) License.
