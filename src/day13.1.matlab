% didn't get a chance to test this out, not idea if it works or not (MATLAB license expired :'( )
b = [17, 23, 35, 36, 40, 52, 54, 95]';
midA = diag([37, 863, 19, 13, 17, 29, 527, 41]);
A = [-23 * ones(size(midA, 1), 1), midA];

n = length(A);
intcon = 1:n;
f = ones(n, 1);
lb = zeros(n, 1);
% options = optimoptions('intlinprog', 'Display', 'off', 'Heuristics', 'none');
ret = intlinprog(f, intcon, [], [], A, b, [], [], options);
% ret = round(ret);
% ret(n+1) = A'*ret(1:n) - v;
