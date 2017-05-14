function checkTests

quant = 25;
% load original image
I = imread('lena.bmp');

% lets find num of tests
command = strcat('waveCompression.exe c');
[q w] = system(command);
numOfTests = str2num(w);

% canonical test
command = strcat('waveCompression.exe 1 lena.bmp test.bmp',32,num2str(quant));
system(command);

%compare images
J = imread('test.bmp');
canPSNR = myPSNR(I,J);
display(strcat('Canonical PSNR = ',32,num2str(canPSNR)));
imshow(J);
pause();

% do all the tests
for i = 2:numOfTests
    command = strcat('waveCompression.exe',32,num2str(i),32,'lena.bmp test.bmp',32,num2str(quant));
    system(command);
    
    %compare images
    J = imread('test.bmp');
    figure();
    imshow(J);
    res = myPSNR(I,J)
    if (res == canPSNR)
        display(strcat('Test',32,num2str(i),32,'is successfull'));
    else
        display(strcat('Test',32,num2str(i),32,'failed'));
    end
end