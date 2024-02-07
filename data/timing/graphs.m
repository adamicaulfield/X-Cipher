clf;
t = csvread("timing-results-others.csv");
t_dense = [t(:,1), t(:,2), (t(:,1)-2).*t(:,1).*t(:,2), t(:,1).*t(:,1).*t(:,2), t(:,5:9)];
a = 1:8;
t = [a', t_dense]
# t = t(1:4,:); # n=5
t = t(5:8,:); #n=7
[hax, H2, H1] = plotyy(t(:,1), t(:,6),t(:,1), t(:,5));
set(H1, 'Color', 'k');
hold on
hold on
H4 = plot(t(:,1), t(:,7));
hold on
H5 = plot(t(:,1), t(:,8));
hold on
H6 = plot(t(:,1), t(:,9));
hold on
H7 = plot(t(:,1), t(:,10));
hold on
ylabel(hax(1), "Execution Time (ms)");
ylabel(hax(2), "Data Size");
xlabel(hax(1), "Iteration");
legend([H1,H2,H4,H5,H6,H7], {"Total Data", "Encrypt and Encode", "Rotate for Recovery", "One Column Recovery", "Two Column Recovery", "Rotate for Use"},'Location','northwest');
set(hax,{'ycolor'},{'k';'k'})
hold on