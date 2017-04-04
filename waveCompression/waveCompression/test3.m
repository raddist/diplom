function test3

t1 = 0.1;
t2 = 0.2;
t3 = 0.3;


% call func to encode first file
command = strcat('waveCompression.exe 3 lena.bmp test3.bmp',32,num2str(t1),32,num2str(t2),32,num2str(t3) );
[v p ] = system(command);

pause(1);

