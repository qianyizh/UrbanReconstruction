function [c, p] = peak(hist, sigma, scale)

nhist = length( hist );
y = normpdf( -5:1:5, 0, sigma );

c = zeros( nhist, 1 );

for i = 1 : nhist
    for j = 1 : 11
        c(i) = c(i) + y(j) * hist( mod(i + j - 7, nhist) + 1 );
    end
end

min_x = min(c);

k = 1;
for i = 1 : nhist
    if ( c(i) >= c(mod(i,nhist) + 1) && c(i) > c(mod(i-2,nhist) + 1) && c(i) > min_x * scale )
        p(k) = pi / nhist * ( i - 0.5 );
        k = k + 1;
    end
end