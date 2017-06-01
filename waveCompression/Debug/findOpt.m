% res = fminsearch(@func, [0.2 1 10]);
% res = res
function [optim_x_min optim_min] = findOpt()
min = 100000000;

%xMin = [0 0 0];
% for j = 1:1:10
%     for k = j + 1:1:15
%         res = func([0.1 j k]);
%         if (res < min)
%             min = res
%             xMin = [0.1 j k]
%         end
%         k = k
%     end
%     j=j
% end

N= 2;
optim_min = zeros(1,N);
optim_x_min = zeros(1,N);
q = 10;

for i = 1:N
    
    xMin = 1;
    for j = 0.1:1:20.1
        res = func(j,q);
        if (res < min)
            min = res;
            xMin = j;
        else
            for k = j:-0.1:j-1
                res = func(k,q);
                if (res < min)
                    min = res;
                    xMin = k;
                end
            end
            break
        end
    end

    optim_min(i) = min;
    optim_x_min(i) = xMin;
    q = q*3/2;
end

