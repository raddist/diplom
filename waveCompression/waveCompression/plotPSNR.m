function plotPSNR

I = imread('lena.bmp');
quant = 27.3;

N = 10;
bpp = ones(N,2);
psnr = ones(N,1);

for i = 1:N
    % call func to encode first file
    command = strcat('waveCompression.exe q lena.bmp test4.bmp',32,num2str(quant) );
    system(command);
    
    pause(1);
    
    %mapped
    fid = fopen('map_encoded.bin', 'rb');
    if fid == -1 
         error('File is not opened'); 
    end
 
    cnt=1;              % ������������� �������� 
    while ~feof(fid)    % ����, ���� �� ��������� ����� ����� 
        [V,N] = fread(fid, 1, 'int8');  %���������� ������ 

        if N > 0        % ���� ������� ��� �������� �������, �� 
        cnt=cnt+1;  % ����������� ������� �� 1 
        end
    end
    
    J = imread('test4.bmp');
    bpp(i,1) = cnt / size(I,1) / size(I,2) * 8;
    psnr(i) = myPSNR(I,J);
    
    % one-model
    fid = fopen('encoded.bin', 'rb');
    if fid == -1 
         error('File is not opened'); 
    end
    
    cnt=1;              % ������������� �������� 
    while ~feof(fid)    % ����, ���� �� ��������� ����� ����� 
        [V,N] = fread(fid, 1, 'int8');  %���������� ������ 

        if N > 0        % ���� ������� ��� �������� �������, �� 
        cnt=cnt+1;  % ����������� ������� �� 1 
        end
    end
    
    bpp(i,2) = cnt / size(I,1) / size(I,2) * 8;
    
    quant = quant*3/2;

    figure();
    imshow(J);
end 

plot(bpp(:,1), psnr,'b', bpp(:,2), psnr,'r');
grid on;
xlabel('bpp');
ylabel('PSNR');
title('image compresion');
legend('256 models','1 model');
end
