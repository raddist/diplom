% res = fminsearch(@func, [0.2 1 10]);
% res = res
function [xMin min] = findOpt()
min = 11000;
xMin = [0 0 0];

for i = 0.1:0.1:10
    for j = 0.1:0.2:20
        for k = 0.1:0.3:30
            res = func([i j k]);
            if (res < min)
                min = res
                xMin = [i j k]
            end
            k = k
        end
        j=j
    end
    i = i
end

min = min
xMin = xMin