pM=36.5951;
KM=1.6665e+03;
Kp01=pM^2/(4*0.1^2*KM);
Kp03=pM^2/(4*0.3^2*KM);
Kp07=pM^2/(4*0.7^2*KM);
Kp1=pM^2/(4*1^2*KM);

numbase=[KM];
denombase=[1 pM 0];
Gabierto=tf(numbase,denombase);
Gabiertored=Gabierto/75;
H01=feedback(G,Kp01);
H03=feedback(G,Kp03);
H07=feedback(G,Kp07);
H1=feedback(G,Kp1);

numinf=[0 0 1];
deninf=[0 0 1];
He=tf(numinf, deninf);



% Caso teórico
subplot(1,2,1); 
step(H07);
xlabel('t');
ylabel('Amplitud');
title('valores teóricos');
legend('lim(t)','con \xi=0,1','con \xi=0,3', 'con \xi=0,707','con \xi=1', 'Location', 'SouthEast');

% Con reduct
subplot(1,2,2); 

 