set dgrid3d 30,30
set hidden3d
set xlabel "Number of Integers (N)"
set ylabel "Message Queue Size (B)"
set zlabel "Average Initialization Time / s"

splot "avginit.dat" u 1:2:3 with lines

pause 100
