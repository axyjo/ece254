binwidth=0.0005
bin(x, width) = width*floor(x/width)

set ylabel "Frequency"
set xlabel "Average Data Transmission Time (n = 500)"

plot 'histogram.dat' using (bin($1,binwidth)):(1.0) smooth freq with boxes


pause 100
