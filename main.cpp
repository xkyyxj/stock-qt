#include "ui/mainwindow.h"
#include <QApplication>
#include <QTextCodec>
#include "utils/zlibcompress.h"
#include <QFile>
#include <iostream>

int main(int argc, char *argv[])
{
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
