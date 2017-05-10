function showQuantedArr()

quant = 25;

% canonical test
command = strcat('waveCompression.exe 1 lena.bmp test.bmp',32,num2str(quant));
system(command);

[arr height width] = loadArrFromFile('quanted.bin');
imshow(arr);

min = arr(1,1);
max = min;
extra_min = 0;
extra_max = 0;
for (i = 1:1:height)
    for (j = 1:1:width)
        if (i < 33 && j < 33)
            if (min > arr(i,j))
                min = arr(i,j);
            end
            if (max < arr(i,j))
                max = arr(i,j);
            end
        else
            if (extra_min > arr(i,j))
                extra_min = arr(i,j);
            end
            if (extra_max < arr(i,j))
                extra_max = arr(i,j);
            end
        end
    end
end

Y = zeros(1, max-min+1);
Y2 = zeros(1, extra_max - extra_min+1);
for (i = 1:1:32)
    for (j = 1:1:32)
       Y(arr(i,j) - min + 1) = Y(arr(i,j) - min + 1) + 1;
    end
end
for (i = 1:1:height)
    for (j = 1:1:width)
        if (i >= 33 || j >= 33)
            if (arr(i,j) ~= 0)
            Y2(arr(i,j) - extra_min + 1) = Y2(arr(i,j) - extra_min + 1) + 1;
            end
        end
    end
end

bar([min:max], Y);
figure;
bar([extra_min:extra_max], Y2);
end
