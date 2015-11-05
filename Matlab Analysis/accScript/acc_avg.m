filename = 'acc10_12_15.xlsx';
A = xlsread(filename);
x = A(:,1);
y = A(:,2);
z = A(:,3);

subplot(2,2,1)
plot(x,'b')
axis([0 size(x,1) -20 20])
title('X')
subplot(2,2,2)
plot(y,'r')
axis([0 size(y,1) -20 20])
title('Y')
subplot(2,2,3)
plot(z,'g')
axis([0 size(z,1) -20 20])
title('Z')

x_avg = 0;
y_avg = 0;
z_avg = 0;
m=[];
for i = 1:size(z)
m(i) = sqrt(x(i)^2+y(i)^2+z(i)^2); 
x_avg = x_avg+abs(x(i));
y_avg = y_avg+abs(y(i));
z_avg = z_avg+abs(z(i));
end  

subplot(2,2,4)
plot(m)
axis([0 size(x,1) 0 30])
title('magnitude')

x_avg = x_avg/size(x,1)
y_avg = y_avg/size(y,1)
z_avg = z_avg/size(z,1)




