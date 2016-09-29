#PURESOFT3D  
[中文版自述点这里](https://github.com/CallMeZhou/Puresoft3D/wiki/Readme-in-Chinese)  
  
##QUICK NEWS
I am half way through making a simple rendering engine based on FBX on top of Puresoft3D, the purposes of which are:
* To enable Puresoft3D rendering more complex scene.
* To test more advanced effects, like mirror reflection, SSR, light scattering, SSAO, etc.
* To make make more fun out of this project.
* To collect more stars  ╮（╯＿╰）╭

##FOR BUSY READERS  
* Rasterisation based software renderer.
* VisualStudio 2010 or later is required (for now).  
* Run initdirs.bat once before building.  
  
##OVERVIEW  
  
*Let's find out what's behind video cards.*  
![](https://qqw6xw.dm2302.livefilestore.com/y3mZX_CzVUMXZNPKuSzZUTCBAvQpa1b1dVV09d3IY35fVN9FezvItAFhZc8iFyHhHgHobFJNKJAOJqDLhCko3c4Msz2duAcpDg-rCpZ0bIbOTZcxwcSMv0zaN1kTdIALUCz7cIYTOSpDwh0ShAe3fU9xg7W1FrkXUKZMr6hJm7-Jgw?width=1024&height=768&cropmode=none)  
This is a software graphics pipeline project. It aims to:  
* *Fullfill personal interest.*  
* *Assist self-paced study.*  
  
As long as you have just learned OpenGL/Direct3D, and are able to draw some fancy stuffs, but your video card is still a damn black box to you, this project fits you well by opening the box up.  
  
Or, if you think you are a fanatical old driver in the pipeline, and you are also keen on challenging me or teaching me lessons, this project fits you too. I opened all the code so I showed all my stupidity in front of you. Never mind being challenged or corrected, playing in no man's land is my only concern.  
  
##HAVE A TASTE?  
  
Well, if you really want to pull out the project to your machine and run the demo, here are some **system minimum requirements**.  
* I wrote and tested the whole project in **VisualStudio 2010 Pro** on **Windows 7 and Vista,** and I really do not have time to port it elsewhere for the time being, so you have to have that environment installed.  
  
* Although the project has minimum dependency on GPU, but you must have a **powerful CPU**. I wish you were a prince from Dubai having a crazy box with hundreds of CPU cores inside. But if you were not, **at lease yours must have 2+ cores**. Mine is an i5 with 2 cores / 4 threads, it draws the above scene in 7 fps, so I guess 4 cores and 8 threads would make about 10-14 fps, that would be more acceptable.  
  
After you open the solution file of the project in VS, you will see 6 projects, among which, the '**Puresoft3D**' is your main course, containing 90% of the pipeline's code. [The WIKI pages have details](https://github.com/CallMeZhou/Puresoft3D/wiki).  
  
You will also see 2 VC-projects named '**test**' and '**test2**', as their names suggest, they are two demo programmes. You may want to set one of them as startup, **switch to release mode and press F5 to build and run it, but before that, be sure to run the initdirs.bat once**. You may noticed the '90%' in the above paragraph. The rest 10% is in the demos --- the shaders, I believe you know it.  
  
'**libobjx**' and '**objcvt**' are two helper tools that converts Wavefront OBJ mesh file to an extended binary format OBJX. They are less important and were actually written 4 years ago when I was doing summer project of master degree courses. The demos load scene from OBJX files which are from OBJ files that I edited in Milkshape. Well, I hate all of them, the OBJ, the OBJX, and Milkshape, but I have to have a way of creating scenes to draw, so I just leave them there and use them. **Just do not spend time on them**.  
  
The last VC-project in the solution is '**mcemaths**' which is from my old school project too, so please do not mind the wired name, 'mce' stands for 'motion capture engine'. This one is important as all linear algebra calculation is implemented in it. However it **may not be important to you** because we have many maths libraries overall the internet, and I believe some of them are much betther than this. About this library, you **just need to know the purposes of the library functions if you want to read the source code in demos as well as the main module. You can get enough description in the mcemaths.h**.  
  
##WHERE TO FIND ME  
  
Send email to [my office mailbox](mailto:chzhoubj@cn.ibm.com) so that I can catch sight of it in every workday's morning. [My personal mailbox](mailto:agedboy@sina.com) is also looking forward to your emails but I usually check it once or twice a week.
