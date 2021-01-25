CREATE TABLE config20 (
	key VARCHAR(10),
	value VARCHAR(10)
);
CREATE TABLE config21 (
	key VARCHAR(10),
	value VARCHAR(10)
);
CREATE TABLE comments (
	key VARCHAR(64),
	value VARCHAR(64),
	date TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

INSERT INTO config20 VALUES ('blursize', '2');

INSERT INTO config21 VALUES ('blursize', '3');

GRANT SELECT ON config20 TO photoshoot;
GRANT SELECT ON config21 TO photoshoot;
GRANT INSERT, SELECT ON comments TO photoshoot;
