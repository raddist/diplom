figure;
grid on; hold on;
line([-1, 1],[0, 0], 'LineWidth',2);
plot(-1,0,'bo','MarkerSize',6); plot(1,0,'bo','MarkerSize',6);

for i=1:5
    line([i, i+1],[i, i], 'LineWidth',2);
    plot(i,i,'b.','MarkerSize',20); plot(i+1,i,'bo','MarkerSize',6);

    line([-i, -i-1],[-i, -i], 'LineWidth',2);
    plot(-i,-i,'b.','MarkerSize',20); plot(-i-1,-i,'bo','MarkerSize',6);
end
