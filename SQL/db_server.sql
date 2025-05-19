DROP TABLE IF EXISTS SystemData;
CREATE TABLE SystemData
(
	private_key TEXT NOT NULL,
	public_key TEXT NOT NULL
);

DROP TABLE IF EXISTS Chat CASCADE;
CREATE TABLE Chat
(
	chat_id INT PRIMARY KEY,
	name VARCHAR(50) NOT NULL,
	picture VARCHAR(200)
);

DROP TABLE IF EXISTS Client;
CREATE TABLE Client
(
    id SERIAL PRIMARY KEY,
    nickname VARCHAR(50) UNIQUE NOT NULL,
	public_key TEXT NOT NULL,
    password VARCHAR(128) NOT NULL,
	picture VARCHAR(200)
);

DROP TABLE IF EXISTS ChatParticipants;
CREATE TABLE ChatParticipants
(
	client_id INT,
	chat_id INT,
	PRIMARY KEY (client_id, chat_id),
	FOREIGN KEY (client_id) REFERENCES Client(id),
	FOREIGN KEY (chat_id) REFERENCES Chat(chat_id)
);

delete from Client;

delete from SystemData;

delete from Client where nickname != 'Dmitry';

select * from SystemData;

select * from Client;

insert into Client
values
	-- (8, 'Ivan', '1234', ''),
	(9, 'misha_Ivan_max', '1234', ''),
	(10, 'sashe', '1234', ''),
	(11, 'da_Iva', '1234', ''),
	(12, 'Ivannavi', '1234', ''),
	(13, 'max', '1234', '');

select * from Client;