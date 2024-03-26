# Agility Potion

This DLL enables Agility SDK support for DirectX 12 games & game engines that can't or won't support it originally.

## What is Agility SDK?

 - **Shipping of new features**

DirectX 12 adds new features frequently, you get a new one like every other month now, but Windows Update either can't or won't keep up in time (Windows 11 has newer versions of DirectX 12 but still may not be the latest versions), not to mention that people may not welcome the presence of Windows Update; This is where Agility SDK comes in, starting with [these versions of Windows 10](https://devblogs.microsoft.com/directx/gettingstarted-dx12agility/#:~:text=For%20Windows%2010,to%20.789), DirectX 12 will check if the game possesses a newer version of its DLLs, namely, `D3D12Core.dll` and `D3D12SDKLayers.dll` *(but don't just go and find those two DLLs and put them in your game folder, it's not that simple and doesn't work that way)*. And if the game has newer versions, the system will load them.

 - **Sounds good, what went wrong?**

However, the ~~ways to integrate Agility SDK~~ **ways for a game to tell the system that it has a newer version of DLLs** may raise some compatibility problems (**which is exactly what this DLL aims to solve**):

1. The Agility SDK requires the game exe to have [two variables](https://devblogs.microsoft.com/directx/gettingstarted-dx12agility/#:~:text=extern%20%22C%22%20%7B%20__declspec(dllexport)%20extern%20const%20UINT%20D3D12SDKVersion%20%3D%20n%3B%7D%0A%0Aextern%20%22C%22%20%7B%20__declspec(dllexport)%20extern%20const%20char*%20D3D12SDKPath%20%3D%20u8%22.%5C%5CD3D12%5C%5C%22%3B%20%7D) "exported" using `__declspec(dllexport)` (which is the way for the game to tell the system its two DLLs' version and location), but since this "export" syntax & this compiler option are exclusive to C/C++, if your main exe is written in any other language you're screwed.

2. Game engines like Unity or Godot physically can't integrate Agility SDK into their engines because of reason 1, and they probably don't care anyway. Unreal Engine lovers here got lucky since UE is written purely in C++.

## For developers?

If you're using Unity Engine, Godot, MonoGame, or any other engine not written in pure C++ and you need some newer DirectX 12 features, then this DLL is right for you for enabling the Agility SDK.

## For gamers?

Let's say that you're on Windows 10, you know for a fact that your graphics card supports the latest DirectX 12 features like ray tracing and variable rate shading and others; you know that the game has those features, but it won't use them and keep saying your PC doesn't have those features; or you may notice that maybe it was using those features, it didn't use the real ones, instead it was using some DirectX 11 fallback alternatives which may be crappy, depending on who implemented those alternative solutions; apply this DLL to your game and it might fix those issues.

In addition, if you find someone online claiming that upgrading to Windows 11 solved those issues, then we're definitely facing the right issue to solve here; you don't need to upgrade to Windows 11 in this case, with this DLL, you don't have to. (*if you did upgrade to Windows 11, you may get a newer version of DirectX 12 without Agility SDK, but it still may not be the **latest**, don't worry though, this DLL should still work on Windows 11... At least in theory.*)

# Installation

You can download the [latest release](https://github.com/wd357dui/Agility-Potion/releases/latest/download/AgilityPotion.zip), unzip it into your game exe's folder (and if you're a developer, ship with the game)

> [!NOTE]
> However, as mentioned above, DirectX 12 adds new features very frequently, thus the latest release of this project may not have been built with the latest Agility SDK; in that case, you'll need to download (clone) this project and build yourself.

# How does this project work?

 - **Background**:

Back a few months ago, I was developing my own DirectX 12 game engine, I came across this Agility SDK thing and wanted to use it. However, I was facing the same issue mentioned above: (in favor of versatility) my main exe is built in C#. (and only the critical low-level implementations are written in C++)... However, as an experienced (single-player major) gamer myself, I've come to be familiar with Cheat Engine pretty well, combining my programming experience, I've come up with this ~~ingenious evil~~ plan to unlock the way to enable the Agility SDK.

- **General principle**:

There is no way for a C# application to have any "export functions (or variables)" like a C/C++ DLL does, but you know what? I don't need it to. I just need to find out how the system finds and reads those "export variables", find out what functions the system uses to do that, and hook those functions, "redirect" them to where I want them to be. Then, it won't matter whether my variable is "exported", I can "force" the system to see what it needs to see, specifically, the pointer address of those two variables. ~~yeeesss... it's all about DECEPTION... hahahahaha~~

 - **Stage 1: "Find out what accesses this address"**

First, I created a C++ desktop application project in Visual Studio, following the original [Getting Started with the Agility SDK](https://devblogs.microsoft.com/directx/gettingstarted-dx12agility/) guide, but before I allowed it to load the `D3D12.dll` dynamically using `LoadLibrary`, I used **Cheat Engine** on my program, added the address for the variables `D3D12SDKVersion` and `D3D12SDKPath`, and then I `right click` -> `Find out what accesses this address`. With this, I've located the code that reads those variables. 

According to the Cheat Engine's comment, the code called the function `GetProcAddress` in `KernelBase.dll` right before it read my variables. That's when I know that I've found the right function to hook.

 - **Stage 2: Hook**

I wrote the `GetProcAddressHook` function, which returns a pointer for my `D3D12SDKVersion` or `D3D12SDKPath` if `lpProcName` matches. At this time I used [Detours](https://github.com/microsoft/Detours) library to hook the GetProcAddress function. ***(Notice that it's the `GetProcAddress` function in `KernelBase.dll`, not `Kernel32.dll`!)***

And then... moment of truth... it works! my breakpoints in `GetProcAddressHook` hits, and later, passing `D3D_FEATURE_LEVEL_12_2` to `D3D12CreateDevice` no longer returns `E_INVALIDARG`! 

 - **Stage 3: Remove third-party dependency**

When I was working on the DirectX hook feature on my mod project [Elements of Harmony](https://github.com/wd357dui/Elements-of-Harmony), I found out that my hook conflicted with the Steam overlay hook (`GameOverlayRenderer64.dll`), Steam is using detour hook on the `IDXGISwapChain::Present` function, while I was using VTable hook. This originally wasn't a problem, but Steam's hook is following some flawed logic: after it had executed its hooked functions, it goes back to call the `IDXGISwapChain::Present` function from the VTable again, it only stops if it finds out that it's calling itself (e.g. the previous frame in the stack trace is itself). But since I added a VTable hook (which means my function is in between the stack frame), that check will always fail, which means it doesn't know when to stop calling itself, ever, which results in a stack overflow. I fixed that by checking for duplicate calls myself (e.g. the function was called 2 times, but has returned 0 times), and temporarily unpatch the detour hook (by looking for & copying the byte code in the original DLL file) when that happens.

While investigating this issue, I checked Steam's detour hook codes and found the code used for jumping to a 64-bit absolute address. Why learn assembly when you can snatch existing code? ~~TIME TO STEAL! HAHAHAHAHA!~~

And that's how I removed dependency on the [Detours](https://github.com/microsoft/Detours) library.
