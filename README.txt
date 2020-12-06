
这个是WDDM-HOOK驱动模拟出一个额外显示终端（虚拟显示器）的代码。
使用WDK7编译。
代码只在WIN7平台某块Intel显卡，某块N卡，以及vmware虚拟显卡测试过。

驱动源代码来源于 xdisp_virt 项目，
xdisp_virt可以去 https://github.com/fanxiushu/xdisp_virt 下载使用。
当初想给xdisp_virt程序增加扩展桌面的功能，
最早想到的是开发一块虚拟显卡，但是基于虚拟总线难以模拟等问题，放弃，
之后想到的是WDDM-HOOK方式来虚拟显示器，虽然在某些显卡上实现了我的需求，
但是没有进行其他多款显卡的测试，
后来找到了更为简单和现成的办法来增加WIN7平台下虚拟显示器，
这个办法在GITHUB上的xdisp_virt项目里面有详细说明，同时CSDN博文里边也有较详细的说明。
因此基本上也把WDDM-HOOK放弃了。

公布的这个WDDM-HOOK源码，基本上是一年多前就做好的，后来也没做什么改进修改。
大体实现了如何HOOK内核函数的办法来模拟出一个虚拟显示器，
测试通过I7 4770K CPU集成的Intel显卡， vmware虚拟机显卡，GT740的N卡。
驱动需要WDK7编译，勿使用WDK10 编译。
至于驱动如何安装使用，以及相关原理，请详细查阅 CSDN文章：
https://blog.csdn.net/fanxiushu/article/details/82731673
 WIN7以上系统WDDM虚拟显卡开发（WDDM Filter/Hook Driver 显卡过滤驱动开发之一） 
 
鉴于wddm-hook的不确定性，因此通常并不帮忙开发定制，也不提供任何支持，
有兴趣可自行下载源码玩玩，没兴趣就当路过凑凑热闹。

另外最近发布的，联系比较紧密的，也是实现虚拟显示器功能的
https://blog.csdn.net/fanxiushu/article/details/93524220
Windows远程桌面开发之九-虚拟显示器（Windows 10 Indirect Display 虚拟显示器驱动开发） 

                                                                                                     fanxiushu  2019

/*********************************************** (baidu translate) *************************************************************

This is the code that the WDDM-HOOK driver simulates an additional display terminal (virtual display).
Compile using WDK7.
The code has only been tested on one Intel graphics card, one n card and VMware virtual graphics card of win7 platform.

The driver source code comes from the xdisp_virt project.
Xdisp_virt can be downloaded at https://github.com/fanxiushu/xdisp_virt.
At first, I wanted to add the function of expanding desktop to xdisp_virt program.
The earliest idea is to develop a virtual graphics card, but based on the virtual bus is difficult to simulate and so on, give up.
Then I think of the WDDM-HOOK virtual display mode, although some graphics cards have fulfilled my requirements.
But no other graphics cards were tested.
Later, a simpler and more ready-made way was found to add virtual displays on the WIN7 platform.
This method is described in detail in the xdisp_virt project on GITHUB and in the CSDN blog.
So WDDM-HOOK is basically abandoned.

The published WDDM-HOOK source code was basically completed more than a year ago, and has not made any improvements since.
In general, how to simulate a virtual display by HOOK kernel function is realized.
Drivers need WDK7 compilation, not WDK10 compilation.
As for how the driver is installed and used, and the related principles, please refer to the CSDN article in detail:
Https://blog.csdn.net/fanxiushu/article/details/82731673

In addition, recently released, more closely linked, but also to achieve the virtual display function.
Https://blog.csdn.net/fanxiushu/article/details/93524220
Windows Remote Desktop Development 9 - Virtual Display (Windows 10 Indirect Display Virtual Display Driver Development)


