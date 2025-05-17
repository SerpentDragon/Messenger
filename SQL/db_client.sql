DROP TABLE IF EXISTS Personal;
CREATE TABLE Personal 
(
    id INT PRIMARY KEY,
    nickname VARCHAR(50) UNIQUE NOT NULL,
	picture VARCHAR(200)
);

DROP TABLE IF EXISTS Contact CASCADE;
CREATE TABLE Contact
(
	contact_id INT PRIMARY KEY,
	nickname VARCHAR(50) UNIQUE NOT NULL,
	picture VARCHAR(200),
	personal_id INT NOT NULL,
	FOREIGN KEY (personal_id) REFERENCES Personal(id)
);

DROP TABLE IF EXISTS Chat CASCADE;
CREATE TABLE Chat
(
	chat_id INT PRIMARY KEY,
	name VARCHAR(50) NOT NULL,
	picture VARCHAR(200),
	personal_id INT NOT NULL,
	FOREIGN KEY (personal_id) REFERENCES Personal(id)
);

DROP TABLE IF EXISTS ChatParticipants;
CREATE TABLE ChatParticipants
(
	contact_id INT,
	chat_id INT,
	PRIMARY KEY (contact_id, chat_id),
	FOREIGN KEY (contact_id) REFERENCES Contact(contact_id),
	FOREIGN KEY (chat_id) REFERENCES Chat(chat_id)
);

DROP TABLE IF EXISTS Message;
CREATE TABLE Message
(
	id SERIAL PRIMARY KEY,
	receiver INT NOT NULL,
	sender INT NOT NULL,
	text VARCHAR(2000) NOT NULL,
	timestamp BIGINT NOT NULL,
	chat_id INT,
	personal_id INT NOT NULL,
	FOREIGN KEY (chat_id) REFERENCES Chat(chat_id),
	FOREIGN KEY (personal_id) REFERENCES Personal(id)
);


delete from Personal;
delete from Contact;
delete from Chat;
delete from ChatParticipants;
delete from Message;

select * from Message;




select * from Personal;

select * from Contact;

insert into Contact
values
	(2, 'Ivan_CL1', ''),
	(3, 'Kirill', ''),
	(4, 'Anna', ''),
	(5, 'Maria', ''),
	(6, 'Ivan_CL2', ''),
	(7, 'Ivan_CL3', '');

insert into Chat
values
	(1, 'Chat 1', ''),
	(2, 'Chat 2', ''),
	(3, 'Chat 3', '');

select * from Contact;
select * from Chat;

select * from Personal;

INSERT INTO Message (receiver, sender, text, timestamp, chat_id) VALUES

(1, 2, 'Пока',                              1715520000000, null),
(2, 1, 'Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!Пока!',                             1715520000000, null),


(1, 2, 'Привет, как дела?',                  1715520000000, null),
(2, 1, 'Привет! Всё хорошо.',                1715520060000, null),

(1, 2, 'Что нового?',                        1715520120000, null),
(2, 1, 'Новости не видел ещё',              1715520180000, null),

(1, 2, 'Когда встретимся?',                 1715520240000, null),
(2, 1, 'Сегодня занят',                      1715520300000, null),

(1, 2, 'А в пятницу свободен?',             1715520360000, null),
(2, 1, 'Пятница подойдёт',                  1715520420000, null),

(1, 2, 'Сходим в кино?',                    1715520480000, null),
(2, 1, 'Кино — отличная идея',              1715520540000, null),

(1, 2, 'Давай в 7?',                         1715520600000, null),
(2, 1, 'Да, давай в 7',                      1715520660000, null),

(1, 2, 'Купишь билеты?',                    1715520720000, null),
(2, 1, 'Я возьму билеты',                   1715520780000, null),

(1, 2, 'Могу перевести деньги',            1715520840000, null),
(2, 1, 'Можешь не переводить',             1715520900000, null),

(1, 2, 'Там новый фильм вышел',            1715520960000, null),
(2, 1, 'Фильм +18',                         1715521020000, null),

(1, 2, 'Напиши, как решишь',               1715521080000, null),
(2, 1, 'Увидимся у входа',                 1715521140000, null),

(1, 2, 'Жду ответа',                        1715521200000, null),
(2, 1, 'Не забудь маску 😷',                1715521260000, null),

(1, 2, 'Или оплатишь сам?',                1715521320000, null),
(2, 1, 'Я возьму на себя',                 1715521380000, null),

(1, 2, 'Ок, до встречи',                   1715521440000, null),
(2, 1, 'Напиши, как выйдешь',              1715521500000, null);


commit;
	