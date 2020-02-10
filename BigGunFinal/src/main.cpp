#include "main.h"

int main()
{
	char com[20] = "/dev/ttyUSB0";
	//SerialPort_Setup(com, 115200);
	
	cout << "加载中..." << endl;
	fsRead.open("xml/armor.xml", FileStorage::READ, "utf-8");
	SvmLoad = StatModel::load<SVM>("xml/svm.xml");
	cout << "获取xml完毕..." << endl;
	
	pthread_t t1, t2, t3;

	pthread_create(&t1, NULL, receive, NULL);
	pthread_create(&t2, NULL, run1, NULL);
	pthread_create(&t3, NULL, run2, NULL);

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	pthread_join(t3, NULL);

	return 0;
}




