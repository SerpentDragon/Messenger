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
	picture VARCHAR(200)
);

DROP TABLE IF EXISTS Chat CASCADE;
CREATE TABLE Chat
(
	chat_id INT PRIMARY KEY,
	name VARCHAR(50) NOT NULL,
	picture VARCHAR(200)
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
	FOREIGN KEY (chat_id) REFERENCES Chat(chat_id)
);

select * from Personal;

insert into Contact
values
	(1, 'Ivan', ''),
	(2, 'Kirill', ''),
	(3, 'Anna', ''),
	(4, 'Maria', '');

insert into Chat
values
	(1, 'Chat 1', ''),
	(2, 'Chat 2', ''),
	(3, 'Chat 3', '');

select * from Contact;
select * from Chat;
	