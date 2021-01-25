#!/bin/sh

su - postgres -c "createuser photoshoot"
su - postgres -c "createuser photoshootowner"
su - postgres -c "createdb -O photoshootowner photoshoot"

cp /srv/photoshoot/db.sql /tmp/db.photoshoot.sql
su - postgres -c "psql -f /tmp/db.photoshoot.sql photoshoot"
rm /tmp/db.photoshoot.sql

touch /srv/photoshoot/setup

