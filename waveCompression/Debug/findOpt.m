% res = fminsearch(@func, [0.2 1 10]);
% res = res
function [xMin min] = findOpt()
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

xMin = 1;
for j = 0.1:0.1:2
    res = func(j);
    if (res < min)
        min = res
        xMin = j
    end
    j = j
end

min = min
xMin = xMin