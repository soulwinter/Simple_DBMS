#include "fileutil.h"

FileUtil::FileUtil()
{

}

bool FileUtil::createFolder(QString path)
{
    QDir dir(path);
    bool ok = dir.mkdir(path);//只创建一级子目录，即必须保证上级目录存在
    return ok;
}
//修改文件夹名
bool FileUtil::modifyFolderName(QString path, QString newName)
{
    QDir dirO(path);
    QDir dir1(newName);
    if (!dirO.exists())
    {
        return false;  //如果原文件夹不存在则返回false
    }
    else
    {
        if (dir1.exists())
        {
            return false;
        }
        else
        {
            dirO.rename(path, newName);
            return true;
        }
    }
}
//删除文件夹
bool FileUtil::deleteFolder(QString path)
{
    QDir dir(path);
    if (!dir.exists())
    {
        return false;  //如果原文件夹不存在则返回false
    }
    else
    {
        dir.removeRecursively();
        return true;
    }
}
//创建文件
bool FileUtil::createFile(QString path)
{
    if (QFile::exists(path))
    {
        return false;
    }
    else
    {
        QFile file(path);
        file.open(QIODevice::WriteOnly);
        file.close();
        return true;
    }
}
//删除文件
bool FileUtil::deleteFile(QString path)
{
    if (!QFile::exists(path))
    {
        return false;
    }
    else
    {
        QFile file(path);
        file.remove();
        file.close();
        return true;
    }
}
//写入整个文件
bool FileUtil::writeAllFile(QString path, char* content)
{
    QFile file(path);
    if (!(file.open(QIODevice::WriteOnly | QIODevice::Truncate)| QIODevice::Text))//QIODevice::Append 替代前两项为增加
        return false;
    file.write(content);
    //QDataStream Stream(&file);
    file.close();
    //Stream << content;
}
//写入整个文件(QString)
bool FileUtil::writeAllFileForQString(QString path, QString content)
{
    QFile file(path);
    if (!(file.open(QIODevice::WriteOnly | QIODevice::Truncate) | QIODevice::Text))//QIODevice::Append 替代前两项为增加
        return false;
    QTextStream stream(&file);
    stream << content;
    file.close();
}
//判断是否存在文件
bool FileUtil::ifExistFile(QString path)
{
    QFileInfo fileInfo(path);
    if (fileInfo.isFile())
    {
        return true;
    }
    return false;
}
//判断是否存在文件夹
bool FileUtil::ifExistFolder(QString path)
{
    QDir dir(path);
    if (dir.exists())
    {
        return true;
    }
    return false;

}
//判断是否有空格
bool FileUtil::ifContainsSpace(QString databaseName)
{
    int i = 0;
    for (i = 0; i < databaseName.length(); i++)
    {
        if (databaseName[i] == ' ')
            return false;
    }
    if (i == databaseName.length())
        return true;
}
//往文件末尾写入一条数据
void FileUtil::writeAtTail(QString path,QString content){
    QFile* file = new QFile(path);
    bool ok = file->open(QIODevice::ReadWrite | QIODevice::Append);
    // 如果文件没有被占用可以打开
    if(ok){
        // 创建stream
        QTextStream txtOutput(file);
        // 在stream追加数据，并换行
        txtOutput << Qt::endl;
        txtOutput << content;
    }
    // 关闭文件, 保存数据
    file->close();
}
//替换文件某处内容
bool FileUtil::replaceContent(QString path,QString oldContent,QString newContent){
    QFile* fileWrite = new QFile(path);
    QFile* fileRead = new QFile(path);
    bool ok = fileRead->open(QIODevice::ReadOnly);
    if(ok){
        QString str(fileRead->readAll());
        if(str.contains(oldContent,Qt::CaseInsensitive)){
            str.replace(oldContent,newContent);
        }
        fileRead->close();
        QFile::remove(path);
        if(fileWrite->open(QIODevice::WriteOnly)){
            fileWrite->write(str.toUtf8());
            fileWrite->close();
            return true;
        }else{
            return false;
        }
    }
    return false;
}
//写入多条记录
bool FileUtil::writeManyAtTail(QString path, vector<QString> content)
{
    QFile file(path);
    if (!(file.open(QIODevice::Append)))//QIODevice::Append 替代前两项为增加
        return false;
    QTextStream stream(&file);
    for (int i = 0; i < content.size(); i++) {
        stream << "\n";
        stream << content[i];
    }
    file.close();
    return true;
}



//读取某一行
QString FileUtil::readLine(QString path, int line)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return "READ ERROR\n";
    QTextStream in(&file);

    for (int i = 0; i < line - 1; i++){
        QString x =in.readLine();
    }


    return in.readLine();
}
//读入全部
QString FileUtil::readAll(QString path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return "READ ERROR\n";
    QTextStream in(&file);
    QString allText = "";
    allText = in.readAll();
    return allText;

}

//删除某一行
void FileUtil::deleteLine(QString path , int line)
{
    /* 获取文件路径 */
    char*  p;
    QByteArray ba = path.toLatin1();
    p=ba.data();

    /* 打开源文件 */
    FILE* fs;
    if ((fs = fopen(p , "r")) == NULL)
    {
        fprintf(stderr , "Couldn't open the file: %s\n" , p);
        exit(EXIT_FAILURE);
    }

    /* 判断该文件一共有几行 */
    int rowNumber = 1;
    for (char c ; feof(fs) == 0;)
    {
        c = getc(fs);
        if (c == '\n')
            rowNumber++;
    }
    rewind(fs);

    /* 打开临时存储用的文件 */
    FILE* fd;
    if ((fd = fopen("D:\\aaa.txt" , "w")) == NULL)
    {
        fprintf(stderr , "first: Couldn't open the d file\n");
        exit(EXIT_FAILURE);
    }

    /* 读取到第 line 行起始处 */
    for (int i = 1 ; i < line ; ++i) //换行执行 line-1 次
    {
        char str[500];
        fgets(str , 100 , fs);
        /* 如果此时要删除的行 为最后一行，且正要读到倒数第二行，则删除倒数第二行末尾的换行符 */
        if (line == rowNumber && i == (line - 1))
        {
            int j;
            for (j = 0 ; str[j] != '\n' ; j++);
            str[j] = '\0';
        }
        fputs(str , fd);
    }

    /* 跳过第line行 */
    char c;
    while (feof(fs) == 0 && (c = getc(fs)) != '\n');


    /* 继续读取到文件末尾 */
    while (feof(fs) == 0)
    {
        char str[100];
        fgets(str , 100 , fs);
        fputs(str , fd);
    }

    /* 关闭两个文件流 */
    fclose(fs);
    fclose(fd);

    /* 将临时文件内容写入源文件 */
    /* 打开源文件 */
    if ((fs = fopen(p , "w")) == NULL)
    {
        fprintf(stderr , "Couldn't open the file: %s\n" , p);
        exit(EXIT_FAILURE);
    }

    /* 打开临时存储用的文件 */
    if ((fd = fopen("D:\\aaa.txt" , "r")) == NULL)
    {
        fprintf(stderr , "second: Couldn't open the d file\n");
        exit(EXIT_FAILURE);
    }

    /* 将临时存储文件写入源文件 */
    while (feof(fd) == 0)
    {
        char str[500];
        fgets(str , 100 , fd);
        fputs(str , fs);
    }

    /* 关闭两个文件流 */
    fclose(fs);
    fclose(fd);

    /* 删除临时文件 */
    remove("D:\\aaa.txt");
    printf("OK!\n");
}

//修改计数器
void FileUtil::updateCounter(QString path,QString num){
    QString allText = readAll(path);
    QString newText = num.append(allText.right(allText.length()-1));
    writeAllFileForQString(path,newText);
}
//检查字段名是否合法
int FileUtil::checkFieldName(QString fieldName){
    //不为空
    if(fieldName.length()==0){
        return STRING_NULL;
    }
    if(!ifContainsSpace(fieldName)){
        return INTEGER_CONTAIN_SPACE;
    }
    return YES;
}
//获取计数器值
int FileUtil::getCounter(QString path) {
    QString str = readLine(path, 1);
    QString num = "";
    for (int j = 0; j < str.length(); j++)
    {
        if (str[j] >= '0' and str[j] <= '9')
            num.append(str[j]);
        else
            break;
    }
    return num.toInt();
}

int FileUtil::getLineByName(QString path, QString Name)
{
    int number = getCounter(path);
    QString str;
    QString name;
    int j = 0;
    for (int i = 2; i < number + 2; i++)
    {
        str = readLine(path, i);
        for (j = 0; j < str.length(); j++)
        {
            if (str[j] >= '0' && str[j] <= '9')
            {}
            else
                break;
        }
        for (j++; j < str.length(); j++)
        {
            if (str[j] != '#')
                name.append(str[j]);
            else
                break;
        }
        if (name == Name)
            return i;
        name = "";
    }
    return 0;
}
