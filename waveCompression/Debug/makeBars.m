% makes bars for all using models
function makeBars(quanted_arr)

models_num = size(quanted_arr, 1);

for i = 1:models_num
   capacity = quanted_arr(i,1);
   if (quanted_arr(i, 1+capacity) + 1 == capacity)
       % model is not used
       continue;
   else
      arr = zeros(1,capacity-1);
      for j = 2:capacity
          arr(1,j-1) = (quanted_arr(i, j+1)-quanted_arr(i, j))/quanted_arr(i, 1+capacity);
      end
      figure;
      bar(0:capacity-2, arr);
   end
end
