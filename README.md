Unreal Engine
=============

This side project was created using the Unreal Engine.


Getting up and running
----------------------

The steps below will take you through cloning your own private fork, then compiling and running the editor yourself:

### Windows

1. Install **[GitHub for Windows](https://windows.github.com/)** then **[fork and clone our repository](https://guides.github.com/activities/forking/)**. 
   To use Git from the command line, see the [Setting up Git](https://help.github.com/articles/set-up-git/) and [Fork a Repo](https://help.github.com/articles/fork-a-repo/) articles.

   If you'd prefer not to use Git, you can get the source with the 'Download ZIP' button on the right. The built-in Windows zip utility will mark the contents of zip files 
   downloaded from the Internet as unsafe to execute, so right-click the zip file and select 'Properties...' and 'Unblock' before decompressing it. Third-party zip utilities don't normally do this.

1. Install **Visual Studio 2017**. 
   All desktop editions of Visual Studio 2017 can build UE4, including [Visual Studio Community 2017](http://www.visualstudio.com/products/visual-studio-community-vs), which is free for small teams and individual developers.
   To install the correct components for UE4 development, check the "Game Development with C++" workload, and the "Unreal Engine Installer" optional component.
  
1. Open your source folder in Explorer and run **Setup.bat**. 
   This will download binary content for the engine, as well as installing prerequisites and setting up Unreal file associations. 
   On Windows 8, a warning from SmartScreen may appear.  Click "More info", then "Run anyway" to continue.
   
   A clean download of the engine binaries is currently 3-4gb, which may take some time to complete.
   Subsequent checkouts only require incremental downloads and will be much quicker.
 
1. Run **GenerateProjectFiles.bat** to create project files for the engine. It should take less than a minute to complete.  

1. Load the project into Visual Studio by double-clicking on the **UE4.sln** file. Set your solution configuration to **Development Editor** and your solution
   platform to **Win64**, then right click on the **UE4** target and select **Build**. It may take anywhere between 10 and 40 minutes to finish compiling, depending on your system specs.

1. After compiling finishes, you can load the editor from Visual Studio by setting your startup project to **UE4** and pressing **F5** to debug.


Licensing and Contributions
---------------------------

Your access to and use of Unreal Engine on GitHub is governed by the [Unreal Engine End User License Agreement](https://www.unrealengine.com/eula). If you don't agree to those terms, as amended from time to time, you are not permitted to access or use Unreal Engine.

We welcome any contributions to Unreal Engine development through [pull requests](https://github.com/EpicGames/UnrealEngine/pulls/) on GitHub. Most of our active development is in the **master** branch, so we prefer to take pull requests there (particularly for new features). We try to make sure that all new code adheres to the [Epic coding standards](https://docs.unrealengine.com/latest/INT/Programming/Development/CodingStandard/).  All contributions are governed by the terms of the EULA.


Additional Notes
----------------

The first time you start the editor from a fresh source build, you may experience long load times. 
The engine is optimizing content for your platform to the _derived data cache_, and it should only happen once.

Your private forks of the Unreal Engine code are associated with your GitHub account permissions.
If you unsubscribe or switch GitHub user names, you'll need to re-fork and upload your changes from a local copy. 

