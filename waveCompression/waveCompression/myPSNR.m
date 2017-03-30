function res = myPSNR( I,J )

imgSize = size(I);
Asignal = 0; Anoise = 0;    %prepare for SNR
for k=1:imgSize(1)
    for l=1:imgSize(2)
        x = double(I(k,l));
        y = double(J(k,l));
        Asignal = Asignal+x^2;
        Anoise = Anoise + (y - x)^2 ;  
    end
end

len = imgSize(1)*imgSize(2);
if (Anoise == 0)
    res = -1;
else
    res = 10 * log10(255*255*len/Anoise);
end


end
