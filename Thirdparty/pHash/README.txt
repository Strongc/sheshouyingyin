ATTENTION:
    Project pHash should be compiled with MinGW. It can not be compiled by using VS 2008.
    
    Soleo Shao
    2011/1/28

pHash Win32 �汾ʹ�� Code::Block 10.05 (MinGW ������) ���б��롣Code::Block(http://www.codeblocks.org/)����Windows��Mac��Linux�µĶ�ƽ̨IDE������������в鿴�ٷ���վ��

����pHash���巽�����£�

1. �ڰ�װ��Code::Block�󣬴򿪱��ļ����µ�pHash.cbp�����ļ���Code::Block����ʶ��������õ������ļ���

2. Ĭ�϶��Ѿ����úã����ֱ�ӱ��뼴����Ŀ¼������libpHash.dll��libpHash.dll.def����libpHash.dll.a�����ļ���

3. ����libpHash.dll �� libpHash.dll.def�ļ�����libpHash.dll.def����ΪlibpHash.def��

4. ʹ�������н���libpHash.def��libpHash.dll����Ŀ¼��ʹ��Visual Studio 2008���ṩ��lib�����������lib�ļ���
   ����Ϊ��lib /machine:i386 /def:libpHash.def /out:libpHash.lib

5. ʹ�øÿ�ʱֻ�����phashapi.h��������TestĿ¼�µ�pHash_Unittest�С�



If you have any problems, please contact splayer.org or Soleo Shao(shaoxinjiang@gmail.com).