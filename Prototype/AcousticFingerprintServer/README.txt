== ��� ==

Acoustic Fingerprint Server ���ڲ�ѯ��¼��phash���Ա�ʶ����Ƶ�ļ�������Ƶ�ļ���

== Acoustic Fingerprint Server Configuration Instruction ==
== AFServer ����˵�� ==

����
========================
1. ����ZeroMQԴ���룬�ڷ������ϰ�װ ZeroMQ 2.1.3 ��ASFserver ���õİ汾��
   
   ����Դ����������л��ZeroMQ��Դ����	
	http://www.zeromq.org/intro:get-the-software

2. ִ�� sh build.sh bin/ ������binΪҪ��װ��Ŀ¼��


����
========================
./afserverd [options]

options: 

-p <portnumber>         Ҫʹ�õĶ˿ں�  - e.g. 5000 -(����)
-w <working dir>        AFServer�Ĺ���·�� default "\tmp"
-b <block size>         blocksize for performing lookup,  default 256
-t <threshold>          threshold for performing lookup, default 0.015
-n <threads>            number of worker threads, default is 60
-i <index name>         path and name of index file -(����)

����: ./afserverd -p 5000 -i /home/soleo/test -n 2


Extra Information
========================

build.sh�еľ��岽��

1. AFServer ������ 3�������⣬�����Ҫ���ڷ������ϰ�װ ZeroMQ �� table �� phashaudio 3���⡣

  ����ʱʹ����AcousticFingerServer/libsĿ¼�е�makefile�� ����˳��Ϊ ZeroMQ ��table ��phashaudio��

2. ʹ��AcousticFingerServerĿ¼�µ�makefile���б��롣

3. �½�POSID.cfg���趨��ʼֵΪ0

4. �����������ļ�����װĿ¼