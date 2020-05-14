#include "ui/mainwindow.h"
#include <QApplication>
#include <QTextCodec>
#include "utils/zlibcompress.h"
#include <QFile>
#include <iostream>
#include <CL/cl.h>

void detect_gpu() {
    cl_platform_id* platforms = nullptr;
    cl_uint num_platforms;
    cl_uint status = clGetPlatformIDs(3, platforms, &num_platforms);
    if(!status) {
        platforms = new cl_platform_id[num_platforms];
        clGetPlatformIDs(3, platforms, &num_platforms);
        std::cout << "PlatForm nums is " << num_platforms << std::endl;
        for(int i = 0;i < num_platforms;i++) {
            char pform_vendor[50];
            clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, sizeof(pform_vendor), &pform_vendor, NULL);
            std::cout << pform_vendor << std::endl;
        }
    }
}

int main(int argc, char *argv[])
{
    detect_gpu();
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
//    zlib::ZLibCompress compress;
//    QFile file("C:\\Users\\Cassiopeia\\Desktop\\222222");
//    file.open(QIODevice::ReadOnly | QIODevice::Text);
//    QByteArray t = file.readAll();
//    char* datas = t.data();
//    compress.startDecompress();
//    std::vector<unsigned char> ret = compress.endDecompress(reinterpret_cast<unsigned char*>(datas), t.size());
//    std::string str(reinterpret_cast<const char*>(ret.data()));
//    std::cout << str << std::endl;

}
