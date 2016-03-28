%rest calculation
filename = 'acc_at_rest.xlsx';
A0 = xlsread(filename);

x0 = A0(:,1);
y0 = A0(:,2);
z0 = A0(:,3);
base_avg = 0;
base=[];
for i = 1:size(z0)
    base(i) = sqrt(x0(i)^2+y0(i)^2+z0(i)^2); 
end  

 baseCorrected = base(100:500);
% mx = max(baseCorrected)
% mn = min(baseCorrected)
 base_avg = mean(baseCorrected)
% dif1 = bmean - mx
% dif2 = bmean - mn
% S = std(baseCorrected)

%threshold calc
S = std(baseCorrected)
lowLimit  = base_avg - 3*S;
highLimit = base_avg + 3*S;

%test case
filename = 'acc10_9_15.xlsx';
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
m_avg = 0;
m=[];
numPeaks = 0; 
flag = 0;

for i = 1:size(z)
    m(i) = sqrt(x(i)^2+y(i)^2+z(i)^2);
    if flag == 0
        if m(i) > highLimit
            flag = 1;
            numPeaks = numPeaks + 1;
        elseif m(i) < lowLimit
            flag = -1;
            numPeaks = numPeaks + 1;
        end
    elseif flag == -1
        if m(i) > highLimit
            flag = 1;
            numPeaks = numPeaks + 1;
        elseif m(i)>lowLimit
            flag = 0;
        end
    elseif flag == 1
        if m(i) < lowLimit
            flag = -1;
            numPeaks = numPeaks + 1;
        elseif m(i) < highLimit
            flag = 0;
        end        
    end
    x_avg = x_avg + abs(x(i));
    y_avg = y_avg + abs(y(i));
    z_avg = z_avg + abs(z(i));
end  
m_avg = mean (m);
subplot(2,2,4)
plot(m)
axis([0 size(x,1) 0 30])
title('magnitude')
%numPeaks
% m_avg = m_avg/size(m,2)
% x_avg = x_avg/size(x,1)
% y_avg = y_avg/size(y,1)
% z_avg = z_avg/size(z,1)




