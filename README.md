# Agility Potion

This DLL enables Agility SDK support for DirectX 12 games & game engines that may or may not originally support it.

## What is Agility SDK?

 - **Shipping of new features**

DirectX 12 adds new features frequently, you get a new one like every other month now, but Windows Update either can't or won't keep up in time (Windows 11 has newer versions of DirectX 12 but still may not be the latest versions), not to mention that people may not welcome the presence of Windows Update; This is where Agility SDK comes in, starting with [these versions of Windows 10](https://devblogs.microsoft.com/directx/gettingstarted-dx12agility/#:~:text=For%20Windows%2010,to%20.789), DirectX 12 will check if the game possesses a newer version of its DLLs, namely, `D3D12Core.dll` and `D3D12SDKLayers.dll` *(but don't just go and find those two DLLs and put them in your game folder, it's not that simple and doesn't work that way)*. And if the game has newer versions, the system will load them.

 - **Sounds good, what went wrong?**

However, the ~~ways to integrate Agility SDK~~ **ways for a game to tell the system that it has a newer version of DLLs** may raise some compatibility problems (which is exactly what this DLL aims to solve):

1. The Agility SDK requires the game exe to have two variables "exported" using `__declspec(dllexport)` (which is the way for the game to tell the system its two DLLs' version and location), but since this "export" syntax & this compiler option are exclusive to C/C++, if your main exe is written in any other language you're screwed.

2. Game engines like Unity or Godot physically can't integrate Agility SDK into their engines because of reason 1, and they probably don't care anyway. Unreal Engine lovers here got lucky since UE is written purely in C++.

## For gamers?

Let's say that you're on Windows 10, you know for a fact that your graphics card supports the latest DirectX 12 features like ray tracing and variable rate shading and others; you know that the game has those features, but it won't use them and keep saying your PC doesn't have those features; or you may notice that maybe it was using those features, it didn't use the real ones, instead it was using some DirectX 11 fallback alternatives which may be crappy, depending on who implemented those alternative solutions; apply this DLL to your game and it might fix those issues.

In addition, if you find someone online claiming that upgrading to Windows 11 solved those issues, then we're definitely facing the right issue to solve here; you don't need to upgrade to Windows 11 in this case, with this DLL, you don't have to. (*if you did upgrade to Windows 11, you may get a newer version of DirectX 12 without Agility SDK, but it still may not be the **latest**, don't worry though, this DLL should still work on Windows 11... At least in theory.*)

## For developers?

If you're using Unity Engine, Godot, MonoGame, or any other engine not written in pure C++ and you need some newer DirectX 12 features, then this DLL is right for you for enabling the Agility SDK.

# Installation

You can download the [latest release](https://github.com/wd357dui/Agility-Potion/releases/latest/download/AgilityPotion.zip), unzip it into your game exe's folder (and if you're a developer, ship with the game)

> [!NOTE]
> However, as mentioned above, DirectX 12 adds new features very frequently, thus the latest release of this project may not have been built with the latest Agility SDK; in that case, you'll need to download (clone) this project and build yourself.
