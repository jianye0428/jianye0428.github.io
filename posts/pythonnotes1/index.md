# Python Notes 1



## python文件相关

### os.path模块

1. **```os.path.exists()```: 判断当前目录以及文件是否存在**
   **```os.path.mkdir()```:  若目录或文件不存在，则创建**
    ```python
    import os
    # 目录
    dirs = '/Users/joseph/work/python/'
    if not os.path.exists(dirs):
        os.makedirs(dirs)
    # 文件
    filename = '/Users/joseph/work/python/poem.txt'
    if not os.path.exists(filename):
        os.system(r"touch {}".format(path))#调用系统命令行来创建文件
   ```
2. **```os.listdir()```： 用于返回指定的文件夹包含的文件或文件夹的名字的列表**
    ```python
    # 打开文件
    path = "/var/www/html/"
    # 如果目录名字为中文 需要转码处理
    path = unicode(path,'utf-8')
    dirs = os.listdir(path)
    # 输出所有文件和文件夹
    for file in dirs:
        print(file)
    ```

3. **```os.path.join()```: 路径拼接**
    ```shell
    import os
    path = "/home"

    # Join various path components
    print(os.path.join(path, "User/Desktop", "file.txt"))
    # /home/User/Desktop/file.txt

    path = "User/Documents"
    # Join various path components
    print(os.path.join(path, "/home", "file.txt"))
    # /home/file.txt
    # In above example '/home'
    # represents an absolute path
    # so all previous components i.e User / Documents
    # are thrown away and joining continues
    # from the absolute path component i.e / home.

    print(os.path.join(path, "Downloads", "file.txt", "/home"))
    # /home
    # In above example '/User' and '/home'
    # both represents an absolute path
    # but '/home' is the last value
    # so all previous components before '/home'
    # will be discarded and joining will
    # continue from '/home'
    ```
4. **```os.path.abspath(path)```: 返回绝对路径**
5. **```os.path.basename(path)```: 返回文件名**
6. **```os.path.commonprefix(list)```: 返回list(多个路径)中，所有path共有的最长的路径**
7. **```os.path.dirname(path)```: 返回文件路径**

8. **```os.path.expanduser(path)```: 把path中包含的"~"和"~user"转换成用户目录**
9. **```os.path.expandvars(path)```: 根据环境变量的值替换path中包含的 "$name" 和 "${name}"**
10. **```os.path.getatime(path)```: 返回最近访问时间(浮点型秒数)**

11. **```os.path.getmtime(path)```: 返回最近文件修改时间**
12. **```os.path.getctime(path)```: 返回文件 path 创建时间**
13. **```os.path.getsize(path)```: 返回文件大小，如果文件不存在就返回错误**

14. **```os.path.isfile(path)```: 判断路径是否为文件**
15. **```os.path.isdir(path)```: 判断路径是否为目录**
16. **```os.path.islink(path)```: 判断路径是否为链接**
17. **```os.path.ismount(path)```: 判断路径是否为挂载点**
18. **```os.path.normcase(path)```: 转换path的大小写和斜杠**
19. **```os.path.normpath(path)```: 规范path字符串形式**
20. **```os.path.realpath(path)```: 返回path的真实路径**
21. **```os.path.relpath(path[, start])```: 从start开始计算相对路径**
22. **```os.path.samefile(path1, path2)```: 判断目录或文件是否相同**
23. **```os.path.sameopenfile(fp1, fp2)```: 判断fp1和fp2是否指向同一文件**
24. **```os.path.samestat(stat1, stat2)```: 判断stat tuple stat1和stat2是否指向同一个文件**
25. **```os.path.split(path)```: 把路径分割成 dirname 和 basename，返回一个元组**
26. **```os.path.splitdrive(path)```: 一般用在 windows 下，返回驱动器名和路径组成的元组**
27. **```os.path.splitext(path)```: 分割路径，返回路径名和文件扩展名的元组**
28. **```os.path.splitunc(path)```: 把路径分割为加载点与文件**
29. **```os.path.walk(path, visit, arg)```: 遍历path，进入每个目录都调用visit函数，visit函数必须有3个参数(arg, dirname, names)，dirname表示当前目录的目录名，names代表当前目录下的所有文件名，args则为walk的第三个参数**
    **```os.walk(path,topdown=True,onerror=None)```: 函数返回一个元组，含有三个元素。这三个元素分别是：每次遍历的路径名、路径下子目录列表、目录下文件列表。**
    ```python
    path = 'xxx/xxx'
    for root, dirs, files in os.walk(path):
        print(root) # path以及path下的目录
        print(dirs) # path下的文件夹
        print(files) # path下每个文件夹中的文件
    ```
    > 区别：```os.path.walk()```与```os.walk()```产生的文件名列表并不相同.os.walk()产生目录树下的目录路径和文件路径，而os.path.walk()只产生文件路径（是子目录与文件的混合列表）。
    > ref: https://www.cnblogs.com/zmlctt/p/4222621.html
30. **```os.path.supports_unicode_filenames```: 设置是否支持unicode路径名**


---

> 作者: [Jian YE](https://github.com/jianye0428)  
> URL: https://lruihao.cn/posts/pythonnotes1/  

