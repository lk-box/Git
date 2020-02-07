使用说明：
    用户只需关心user.cpp、user.h、main.cpp这三个文件。

user.h：
    控制宏：
        #define CAMER_NUM (1) 用于选择单目与双目模式 1为单目模式 2为双目模式

      	#define SHOW_PIC 控制是否显示图像，一般调试模式时显示图像，上场运行时注释此宏
	#define SHOW_AllPIC  控制是否让所有的线程都显示图像。图像处理是以多线程的方式进行的，如果定义了这个宏的话每个线程都会单独的创建窗口用于显示图像（showPic()函数的实现）。这是因为opencv的imshow() 函数无法保证在多线程的状态下在同一个窗口显示图像，为了保证用户能够看到图像处理过程中的每一幅图像才出此下策。如果用户关闭这个宏，则每两幅图像中将只会显示一副。这是一个缺陷，待后续了解opencv　imshow() 函数的实现后可以进行优化。



        #define USING_UART2 选择是否使用串口2 。未注释为“使用”，注释掉后为“不使用”。

        #define UART2_NAME ("/dev/ttyTHS1")
        #define UART2_BAUD (115200)
        以上两个宏用于设置串口2的“文件名”以及波特率。在Linux下，串口2所对应的设备文件的名称即为：/dev/ttyTHS1
        串口3同串口2

        #define MAX_MSGLEN 100 串口通信中单条消息的最大长度（单位：字节）。这个值大一点无所谓。

user.cpp
    接口函数：
            void uart2Handler(char *data) 串口2数据处理函数 data为串口2接收到的数据
            void uart3Handler(char *data) 串口3数据处理函数 data为串口3接收到的数据
            void pic_dispose(Mat &frame)  单目模式下的图像处理函数 frame为摄像头输出的原始图像
            void pic_dispose_2(Mat &frame_1, Mat &frame_2) 双目模式下的图像处理函数。frame_1、frame_2为两个摄像头的原始图像
    工具函数：
            void uart2Send(const char *buf) 串口2数据发送
            void uart3Send(const char *buf) 串口3数据发送
            void show_pic(const char *msg, Mat frame，int tid) 显示图像，将opencv的imshow封装了一下，以适应多线程环境下的图像显示。
	！！注意：显示图像，必须使用show_pic这个工具函数，不能直接使用opencv的imshow()，否则可能导致程序崩溃。

main.cpp
        allChannel->Uart2->enableRecive(); 串口2接收使能，注释掉这句话后串口2就不会接收数据了。这句话也可以放在其他地方。
	其他的在代码注释中已经进行了说明。
　　
	串口通信的通信协议：
          stm32向妙算发送数据时需要加帧头($)，帧尾(#).例如：32想要向妙算发送数据：abc123，在实际发送时应该发送：$abc123#,妙算接收到后会进行帧头帧尾校验，并将有效数据"abc123"返回给用户,即uartXHandler(char *data)函数中data的值为"abc123"。
对于串口通信的实现，暂时可以理解为类似于32中的串口中断,用户只需要在uartXHandler(char *data)函数中写数据处理的代码即可。


Makefile使用：
       妙算新开机后由于系统时间不对，在编译前执行一遍：make clean 
       以后每次更改代码后直接执行make即可
       新添加的源文件(.cpp .c)放在src中，头文件放在include_u中。无需更改Makefile。
       需要连接新的库是将库名添加到Makefile中的第8行：USER_LIB = -lpthread -lv4l2 的后边。
	所有opencv的库都已包含（opencv 2.0/3.0)在执行make前自行根据机器上已装的opencv版本在Makefile中选择（如果机器上装的是opencv2则注释第７行，如果是oopencv3则注释第９行。


