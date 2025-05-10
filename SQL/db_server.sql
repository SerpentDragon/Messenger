DROP TABLE IF EXISTS Client;
CREATE TABLE Client
(
    id SERIAL PRIMARY KEY,
    nickname VARCHAR(50) UNIQUE NOT NULL,
    password VARCHAR(128) NOT NULL,
	picture VARCHAR(200)
);


delete from Client;

insert into Client
values
	(2, 'Ivan', '1234', ''),
	(3, 'misha_Ivan_max', '1234', ''),
	(4, 'sashe', '1234', ''),
	(5, 'da_Iva', '1234', ''),
	(6, 'Ivannavi', '1234', ''),
	(7, 'max', '1234', '');

select * from Client;