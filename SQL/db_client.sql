DROP TABLE IF EXISTS Personal CASCADE;
CREATE TABLE Personal 
(
    id INT PRIMARY KEY,
    nickname VARCHAR(50) UNIQUE NOT NULL,
	private_key TEXT NOT NULL,
	public_key TEXT NOT NULL,
	picture VARCHAR(200)
);

DROP TABLE IF EXISTS Contact CASCADE;
CREATE TABLE Contact
(
	contact_id INT,
	nickname VARCHAR(50) NOT NULL,
	public_key TEXT NOT NULL,
	picture VARCHAR(200),
	personal_id INT NOT NULL,
	PRIMARY KEY (contact_id, personal_id),
	FOREIGN KEY (personal_id) REFERENCES Personal(id)
);

DROP TABLE IF EXISTS Chat CASCADE;
CREATE TABLE Chat
(
	chat_id INT NOT NULL,
	name VARCHAR(50) NOT NULL,
	picture VARCHAR(200),
	personal_id INT NOT NULL,
	PRIMARY KEY (chat_id, personal_id),
	FOREIGN KEY (personal_id) REFERENCES Personal(id)
);

DROP TABLE IF EXISTS ChatParticipants;
CREATE TABLE ChatParticipants
(
	contact_id INT,
	chat_id INT,
	personal_id INT,
	PRIMARY KEY (contact_id, chat_id, personal_id),
	FOREIGN KEY (contact_id, personal_id) REFERENCES Contact(contact_id, personal_id),
	FOREIGN KEY (chat_id, personal_id) REFERENCES Chat(chat_id, personal_id)
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
	vanishing BOOL,
	personal_id INT NOT NULL,
	FOREIGN KEY (chat_id, personal_id) REFERENCES Chat(chat_id, personal_id),
	FOREIGN KEY (personal_id) REFERENCES Personal(id)
);





select * from Contact;

SELECT * FROM 
                (SELECT DISTINCT ON (text, sender) * FROM Message ORDER BY text, sender)
                WHERE chat_id = 38 AND personal_id = 1
                ORDER BY timestamp;

delete from Personal;
delete from Contact;

delete from ChatParticipants;
delete from Message;
--delete from Contact;
delete from Chat;

select contact_id, nickname, personal_id from Contact;

delete from ChatParticipants;
delete from Chat;

select contact_id, nickname, personal_id from Contact order by personal_id;

select * from Chat;
select * from ChatParticipants;

select * from Message;

SELECT * FROM
                (SELECT DISTINCT ON (text, sender) * FROM Message WHERE personal_id = 2 ORDER BY text, sender)
                WHERE chat_id = 100
                AND personal_id = 2
                ORDER BY timestamp;

select * from Personal;

SELECT * FROM
	(SELECT DISTINCT ON (text, sender) *
	FROM Message, Contact AS C_R, Contact AS C_S
	ORDER BY text, sender)
WHERE chat_id = 38 AND personal_id = 1
ORDER BY timestamp


select * from Personal;


-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA2/aadyY0pBAiKZmJQRGp
NtFPUlWUdu5byXmgLQJQmCYkNhxmqj5UzB3kXTomGXTgsBsl4mXagcBGMO/k79TU
XeJM30J4ePhosaGhLTuVLmUO4DLUKNyTZz9lysN5o3UYHoOTJqB4xC6xT/K4tiAS
djN9IZGsk9G2yG8vsTEMbaZKaCbkNKpp7r2DK3zaiuRtvf9ljeChiLJyiFXtHAGq
eL9PQYDNBdq1JrcwgN0GppQb6+xzRNp3Luw0qExdsKJ6Uj85AfAkUG+CmYcl0P6J
0oeBbfqQsZVAe4YP6AihauQ5hWG1eCRKP10Ow/nGCTyGBRkJ981/PdnEPw5369yA
MQIDAQAB
-----END PUBLIC KEY-----


-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAqXg0n5QcgPuREJPTT/Ta
oP0K5tvGn/a8MmLPjAnrr0V4K5n+4eIx8itr5tQEUNUgVaolXScsmXZOOiANjjBE
qGrGdH/QO6ysw+FItQJAdDg/8nGTreqaAeARW5I4zAJPKcW7tzRD6fkD8Xdt9VEI
qH7E5unzZQ0lpe7DQE9NEFx0E5A8+FykyVqNYm9Lql9IA7m30OZJbncEszKMZT14
53GumCUIFMr5uZ6juDrM4lFzAbrS4CW1D/V9Gj68fji5GW8fo2T4azXtaM149DQA
8BLUaZQLkpGBAUL7QBxNAecqBfwDA0cXxUXEPIeGHEIi5dwo8ICQEImGPB2sTmMY
AQIDAQAB
-----END PUBLIC KEY-----

-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAqFJyVHSPHLakPYe6ADTy
4JnUKmi3ZNaBsTWGQaw8fY/6pzlsfuJh0fWy7lIW+ZCvw0hFVlBMcE8GH9yWN0xD
hm5Rs759R5YXKOQpbhuLUqqeyNZxD9aDmMPxv4xQ0OX+wh/3lmK9Ccg0MhutuBzH
fQltYJfy1lTqKjy0/2SYIdDtUR+d+YBn+whcjrWXiMOKLTgiFnqn6S4fXstXRMAv
b3wdBi4OQ8PuanBcFlsxhFs/qLTLLlnAj5U2hl+8YcI+r7C4FaGs7DA9lNxOOapU
KoxdGl7QKDZXjHqeoQu6PBBM7tC5IwHSp1yG8svbv4621QWtK1GzKAgeoFBYYKqE
cQIDAQAB
-----END PUBLIC KEY-----


select * from Contact;

delete from Contact where contact_id = -1;


-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAqFJyVHSPHLakPYe6ADTy
4JnUKmi3ZNaBsTWGQaw8fY/6pzlsfuJh0fWy7lIW+ZCvw0hFVlBMcE8GH9yWN0xD
hm5Rs759R5YXKOQpbhuLUqqeyNZxD9aDmMPxv4xQ0OX+wh/3lmK9Ccg0MhutuBzH
fQltYJfy1lTqKjy0/2SYIdDtUR+d+YBn+whcjrWXiMOKLTgiFnqn6S4fXstXRMAv
b3wdBi4OQ8PuanBcFlsxhFs/qLTLLlnAj5U2hl+8YcI+r7C4FaGs7DA9lNxOOapU
KoxdGl7QKDZXjHqeoQu6PBBM7tC5IwHSp1yG8svbv4621QWtK1GzKAgeoFBYYKqE
cQIDAQAB
-----END PUBLIC KEY-----





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
	