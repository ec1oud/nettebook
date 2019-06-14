create sequence journal_id_s increment 1 start 1;
create sequence journal_type_s increment 1 start 1;
create table journal_type (description varchar(80), last_mod timestamp default 'now', type int4 not null default nextval('journal_type_s') primary key);
create table journal (owner_id int4, type int4, data varchar(80), last_mod timestamp default 'now', id int4 not null default nextval('journal_id_s') primary key);
create sequence gp_id_s increment 1 start 1;
create table general_purpose (owner_id int4, category varchar(40), title varchar(80), body text, last_mod timestamp default 'now', gp_id int4 not null default nextval('gp_id_s') primary key);
create sequence entity_id_s increment 1 start 1;
create table thing (name varchar(160), type int4, owner int4, last_mod timestamp default 'now', entity_id int4 not null default nextval('entity_id_s') primary key);
create sequence thing_type_s increment 1 start 1000;
create table thing_type (name varchar(40), type_id int4 not null default nextval('thing_type_s'), default_relationship int4 not null);
create table contact_method (entity_id int4 not null, location_id int4, type varchar(10), description varchar(20), detail varchar(2048), last_mod timestamp default 'now');
create sequence conversation_id_s increment 1 start 1;
create table conversation (owner_id int4 not null, other_id int4 not null, comment text, beginning timestamp, ending timestamp default 'now', conversation_id int4 not null default nextval('conversation_id_s') primary key);
create sequence entity_comment_id_s increment 1 start 1;
create table entity_comment (entity_id int4 not null, comment text, last_mod timestamp default 'now', comment_id int4 not null default nextval('entity_comment_id_s') primary key);
create table category (owner_id int4, name varchar(255), default_document_type int2, default_document_encoding int2, parent_category int4 default null, id int4 not null default nextval('entity_id_s') primary key);
create table categorization (owner_id int4, category_id int4, entity_id int4 primary key);
create table document (owner_id int4, title varchar(255), body text, encoding_id int2, type_id int2, last_mod timestamp default 'now', id int4 not null default nextval('entity_id_s') primary key);
create sequence location_id_s increment 1 start 1;
create table location (name varchar(80), description varchar(1024), address varchar(1024), entity_id int4, last_mod timestamp default 'now', location_id int4 not null default nextval('location_id_s') primary key );
create table location_assoc (entity_id int4 not null, location_id int4 not null);
create table person (lastname varchar(80), othernames varchar(120), suffixes varchar(40), last_mod timestamp default 'now', entity_id int4 not null default nextval('entity_id_s') primary key, primary_comment int4, primary_employer int4);
create table relationship_assoc (one_entity int4 not null, other_entity int4 not null, type_id int4, comment text, primary key (one_entity, other_entity));
create sequence relationship_type_id_s increment 1 start 1000;
create table relationship_type (name varchar(40), sentence_usage varchar(80), reverse_sentence_usage varchar(80), type_id int4 not null default nextval('relationship_type_id_s') primary key);
create table "user" (userid varchar(8) not null, password varchar(20), pda_userid varchar(20), pda_password varchar(20), pda_last_sync timestamp, entity_id int4 not null);
create table user_prefs (entity_id int4 not null, name varchar(20), value varchar(80), type char, primary key(entity_id, name));
create sequence template_id_s increment 1 start 1;
create table mailmerge_template (name varchar(80), template text, creator int4, last_mod timestamp default 'now',  template_id int4 not null default nextval('template_id_s') primary key);
create table list_addresses (member_id int4, thing_id int4, location_id int4);
CREATE SEQUENCE "image_type_s" start 1000 increment 1;
CREATE TABLE "image_type" (
        "name" varchar(20),
        "type_id" int4 DEFAULT nextval ( 'image_type_s' ) NOT NULL);
CREATE TABLE "image" (
        "entity_id" int4,
        "name" varchar(80),
        "caption" text,
        "type" int4,
        "filename" varchar(80),
        "thumbnail" varchar(80),
        "id" int4 DEFAULT nextval ( 'entity_id_s' ) NOT NULL PRIMARY KEY);
CREATE TABLE "output_device" (
        "id" int4,
        "name" varchar(80),
        "commandline" varchar(255),
        "type" int4);
create table mailmerge_history (member_id int4, thing_id int4, location_id int4, merge_time timestamp default 'now');
grant all on thing, thing_type, contact_method, conversation, entity_comment, general_purpose, location, location_assoc, person, relationship_assoc, relationship_type, "user", user_prefs, list_addresses, image_type, image, output_device, mailmerge_template, mailmerge_history to wwwdata;

insert into relationship_type values('employment', 'works for', 'employs', 1);
insert into relationship_type values('love', 'loves', 'is loved by', 2);
insert into relationship_type values('hate', 'hates', 'is hated by', 3);
insert into relationship_type values('fatherhood', 'is the father of', 'is the child of', 4);
insert into relationship_type values('motherhood', 'is the mother of', 'is the child of', 5);
insert into relationship_type values('marriage', 'is married to', 'is the spouse of', 6);
insert into relationship_type values('membership', 'is a member of', 'has in its membership', 7);
insert into relationship_type values('cohabitation', 'lives with', 'lives with', 8);
insert into relationship_type values('ownership', 'owns', 'belongs to', 9);
insert into thing_type values('company', 1, 1);
insert into thing_type values('organization', 2, 7);
insert into thing_type values('mailing list', 3, 7);
insert into thing_type values('property', 4, 9);
insert into thing_type values('financial asset', 5, 9);
INSERT INTO "output_device" VALUES (0,'downloadable file',NULL,0);
INSERT INTO "output_device" VALUES (1,'printer','/usr/bin/lpr',1);
INSERT INTO "output_device" VALUES (2,'printer, via LaTeX','/usr/local/bin/texprint',1);
create sequence document_type_s increment 1 start 1000;
create table document_type (name varchar(40), mime_type varchar(20), id int2 not null default nextval('document_type_s'));
insert into document_type values('ASCII', '', 0);
insert into document_type values('DWIM/structured text', '', 1);
insert into document_type values('HTML', 'text/html', 2);
insert into document_type values('XML', 'text/xml', 3);
insert into document_type values('LaTeX', '', 4);
insert into document_type values('Postscript', '', 5);
create sequence encoding_s increment 1 start 1000;
create table encoding (name varchar(40), description varchar(80), shortcut char(1) unique, id int2 not null default nextval('encoding_s'));
insert into encoding values('ISO 8859-1', 'Latin 1; Western European languages', '1', 1);
insert into encoding values('ISO 8859-2', 'Latin 2; Slavic/Central European languages', '2', 2);
insert into encoding values('ISO 8859-3', 'Latin 3; Esperanto, Galician, Maltese, Turkish', '3', 3);
insert into encoding values('ISO 8859-4', 'Latin 4; Estonian, Latvian, Lithuanian', '4', 4);
insert into encoding values('ISO 8859-5', 'Latin 5; Cyrillic', '5', 5);
insert into encoding values('ISO 8859-6', 'Arabic', 'a', 6);
insert into encoding values('ISO 8859-7', 'Modern Greek', 'g', 7);
insert into encoding values('ISO 8859-8', 'Hebrew', 'h', 8);
insert into encoding values('ISO 8859-9', 'Latin 5; Turkish', 't', 9);
insert into encoding values('ISO 8859-10', '', null, 10);
insert into encoding values('ISO 8859-11', '', null, 11);
insert into encoding values('ISO 8859-12', '', null, 12);
insert into encoding values('ISO 8859-13', '', null, 13);
insert into encoding values('ISO 8859-14', 'Latin 8; Celtic', 'c', 14);
insert into encoding values('ISO 8859-15', 'Latin 9; Western European languages with Euro', 'e', 15);
insert into encoding values('KOI8-R', 'Russian', 'r', 16);
insert into encoding values('unicode', 'unicode', 'u', 100);
insert into encoding values('UTF8', 'UTF 8/16 bit', '8', 101);
insert into encoding values('binary', '8 bit non-human-readable', 'b', 102);
