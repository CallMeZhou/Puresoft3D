# PURESOFT3D  
[中文版自述点这里](https://github.com/CallMeZhou/Puresoft3D/wiki/Readme-in-Chinese)  

**IMPORTANT:** THE DEVELOPMENT OF THIS PROJECT HAS PERMANENTLY STOPPED. SINCERE APPRECIATION FOR ALL STAR-GIVERS. YOU GUYS HAVE GIVEN ME REAL HAPPYNESS.

Believe me, I still love this project and the rasterization technologies. However, as a middle-aged man, I have to spend more time on my family.

I am truly sorry that this project still has too many imperfactions, which include but not limited to:

1. It is tightly bound to VisualStudio. Nowadays people use cmake.
2. It is tightly bound to Windows indeed.
3. It uses Intel style inline assmeber that makes it hard to port to other platforms.
4. It has a over complicated shader classes hierachy that make it hard to use.
5. The objx file format is a real shit!
6. The field of depth (DOF) effect is eventually not included. (the screenshot was produced by my experimental code that was not pushed)
7. The scene/stage file and the scene editor were eventually not implemented (crying).

Well... but my life is short and so is my wife's. I can't keep her waiting any more. I have to share the rest of my life with her.
  
## OVERVIEW  
  
* Let's find out what's behind video cards.*  
![](https://github.com/CallMeZhou/Puresoft3D/blob/master/wikires/cover.jpg)  
This is a software graphics pipeline project. It aims to:  
* *Fullfill personal interest.*  
* *Assist self-paced study.*  
  
Perhaps you learned something about OpenGL/Direct3D, and are able to draw fancy stuffs with it too, but your video card is still a damn black box to you. Well, this project fits you well by opening the box up.  
  
If this is not the case and you think you are an old driver in the pipeline, and you are also keen on challenging me or teaching me lessons, this project would be interesting to you too. I opened all the code so I showed all my stupidity in front of you. Never mind being challenged or corrected, playing with nobody is my only concern.  
  
## HAVE A TASTE?  
  
Well, if you really want to pull out the project to your machine and run the demo, here are some **system minimum requirements**.  
* I wrote and tested the whole project in **VisualStudio 2010 Pro** on **Windows 7 and Vista,** and I really do not have time to port it elsewhere for the time being, so you have to have that environment installed.  
  
* Although the project has minimum dependency on GPU, but you must have a **powerful CPU**. I wish you were a prince from Dubai having a crazy box with hundreds of CPU cores inside. But if you are not, **at lease yours must have 2+ cores**. Mine is an i5 with 2 cores / 4 threads, it draws the above scene in 7 fps, so I guess 4 cores and 8 threads would make about 10-14 fps, that would be more acceptable.  
  
After you open the solution file of the project in VS, you will see 6 projects, among which, the '**Puresoft3D**' is your main course, containing 90% of the pipeline's code. [The WIKI pages have details](https://github.com/CallMeZhou/Puresoft3D/wiki).  
  
You will also see 2 VC-projects named '**test**' and '**test2**', as their names suggest, they are two demo programmes. You may want to set one of them as startup, **switch to release mode and press F5 to build and run it, but before that, be sure to run the initdirs.bat once**. You may noticed the '90%' in the above paragraph. The rest 10% is in the demos --- the shaders, I believe you know it.  
  
'**libobjx**' and '**objcvt**' are two helper tools that converts Wavefront OBJ mesh file to an extended binary format OBJX. They are less important, written 4 years ago during my school project. The demos load scene from OBJX files which are from OBJ files that I edited in Milkshape. Well, **I hate all of them**, the OBJ, the OBJX, and Milkshape, but I have to have a way of creating scenes to draw, so I just leave them there and use them. **Just do not waste time on them**.  
  
The last VC-project in the solution is '**mcemaths**' which is from my old school project too, so please do not mind the wired name prefix. This module is important to the project as all linear algebra calculation is implemented in it. However it **may not be important to you** because we have many maths libraries all over the internet, and I believe some of them are much betther than this one. About this library, you **just need to know the purposes of the library functions if you want to read the source code in demos as well as the main module. You can get enough description in the mcemaths.h**.  
  
## WHERE TO FIND ME  
  
Send email to [My personal mailbox](mailto:agedboy@sina.com). I usually check it once or twice a week.
