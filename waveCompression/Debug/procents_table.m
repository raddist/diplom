function [res res2] = procents_table(psnr, bpp)

test_num = size(psnr,1);
res = zeros(5, test_num);
res2 = zeros(2, test_num)

% fill psnr
for  i = 1:test_num
    res(1, i) = psnr(i);

% fill base ar coding
    res(2, i) = bpp(i,2) / bpp(i,2);

% fill mem arcod
    res(3, i) = bpp(i,1) / bpp(i,2);

% fill context arcod
    res(4, i) = bpp(i,3) / bpp(i,2);

% fill sign context
    res(5, i) = bpp(i,5) / bpp(i,2);
    
    res2(1,i) = psnr(i);
    res2(2,i) = bpp(i,5) / bpp(i,3) * 100;
end
