DROP TABLE IF EXISTS SystemData;
CREATE TABLE SystemData
(
	private_key TEXT NOT NULL,
	public_key TEXT NOT NULL
);

DROP TABLE IF EXISTS Chat CASCADE;
CREATE TABLE Chat
(
	chat_id SERIAL PRIMARY KEY,
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
