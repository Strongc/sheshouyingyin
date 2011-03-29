ATTENTION:
    Project pHash should be compiled with MinGW. It can not be compiled by using VS 2008.
    
    Soleo Shao
    2011/1/28

pHash Win32 版本使用 Code::Block 10.05 (MinGW 编译器) 进行编译。Code::Block(http://www.codeblocks.org/)是在Windows、Mac、Linux下的多平台IDE，具体介绍自行查看官方网站。

编译pHash具体方法如下：

1. 在安装完Code::Block后，打开本文件夹下的pHash.cbp。该文件是Code::Block用于识别各种设置的配置文件。

2. 默认都已经设置好，因此直接编译即可在目录中生成libpHash.dll、libpHash.dll.def、和libpHash.dll.a三个文件。

3. 拷贝libpHash.dll 、 libpHash.dll.def文件。将libpHash.dll.def改名为libpHash.def。

4. 使用命令行进入libpHash.def、libpHash.dll所在目录。使用Visual Studio 2008所提供的lib命令进行生成lib文件。
   命令为：lib /machine:i386 /def:libpHash.def /out:libpHash.lib

5. 使用该库时只需包含phashapi.h。例程在Test目录下的pHash_Unittest中。



If you have any problems, please contact splayer.org or Soleo Shao(shaoxinjiang@gmail.com).