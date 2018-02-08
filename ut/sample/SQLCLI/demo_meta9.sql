drop table lines;
drop table orders;
drop table customers;

create table customers(
custid integer primary key,
name char(50),
address varchar(100),
phone char(10));

create table orders(
orderid integer primary key,
custid integer,
opendate date,
salesperson char(50),
status integer,
foreign key (custid) references customers (custid));

create table lines(
orderid integer,
lines integer,
partid integer,
quqntity integer,
foreign key (orderid) references orders (orderid));

