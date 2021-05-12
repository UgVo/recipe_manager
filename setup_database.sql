CREATE TABLE element_types (
	name text, 
	CONSTRAINT PK_elementName PRIMARY KEY(name)
);

CREATE TABLE process_types ( 
	name text, 
	CONSTRAINT PK_processName PRIMARY KEY(name)
);

CREATE TABLE equipments (
	name text, 
	CONSTRAINT PK_equipementName PRIMARY KEY(name)
);

CREATE TABLE ingredients (
	id SERIAL UNIQUE, 
	type text, 
	name text, 
	recipe_id integer, 
	CONSTRAINT PK_ingredient PRIMARY KEY(id), 
	CONSTRAINT FK_typeName FOREIGN KEY (type) REFERENCES element_types (name) ON DELETE CASCADE
);

CREATE INDEX pkid_ingredient ON ingredients (id);

CREATE TABLE processes (
	id SERIAL UNIQUE, 
	type text, 
	duration integer, 
	temperature integer, 
	recipe_id integer,
	step_id integer,
	CONSTRAINT PK_process PRIMARY KEY(id), 
	CONSTRAINT FK_processType FOREIGN KEY (type) REFERENCES process_types (name) ON DELETE CASCADE,
	CONSTRAINT FK_recipeId FOREIGN KEY (recipe_id) REFERENCES recipes (id) ON DELETE CASCADE,
	CONSTRAINT FK_stepId_processes FOREIGN KEY (step_id) REFERENCES steps (id) ON DELETE CASCADE
);
CREATE INDEX pkid_processes ON processes (id);
CREATE INDEX fkrecipe_id_processes ON processes (recipe_id);
CREATE INDEX fkstep_id_processes ON processes (step_id);

CREATE TABLE milestones (
	id SERIAL UNIQUE, 
	rank integer, 
	name text,
	recipe_id integer, 
	CONSTRAINT PK_milestones PRIMARY KEY(id), 
	CONSTRAINT FK_recipe FOREIGN KEY(recipe_id) REFERENCES recipes(id) ON DELETE CASCADE
);
CREATE INDEX pkid_milestones ON milestones (id);
CREATE INDEX fkrecipe_id_milstones ON milestones (recipe_id);

CREATE TABLE recipes (
	id SERIAL UNIQUE, 
	image_url text,
	setting_up_image_url text, 
	servings integer,  
	CONSTRAINT PK_recipe PRIMARY KEY(id), 
);
CREATE INDEX pkid_recipes ON recipes (id);

CREATE TABLE steps (
	id SERIAL UNIQUE,  
	rank integer, 
	description text, 
	imagesurls text,  
	milestone_id integer, 
	CONSTRAINT PK_step PRIMARY KEY(id),
	CONSTRAINT FK_milestone FOREIGN KEY (milestone_id) REFERENCES milestones(id) ON DELETE CASCADE
);
CREATE INDEX pkid_steps ON steps (id);
CREATE INDEX fkmilestone_id ON steps (milestone_id);

CREATE TABLE rel_step_equi (
	step_id integer,
	equipement text,
	CONSTRAINT PK_re_step_equi PRIMARY KEY (step_id,equipement),
	CONSTRAINT FK_step_rel FOREIGN KEY (step_id) REFERENCES steps(id) ON DELETE CASCADE,
	CONSTRAINT FK_equi_rel FOREIGN KEY (equipement) REFERENCES equipments(name) ON DELETE CASCADE
);
CREATE INDEX pkfkstep_id_rel ON rel_step_equi (step_id);
CREATE INDEX pkfkequipement_rel ON rel_step_equi (equipment);

CREATE TABLE components (
	id SERIAL UNIQUE, 
	ingredient_id integer, 
	quantity integer, 
	unit integer, 
	step_id integer;
	CONSTRAINT PK_component PRIMARY KEY(id), 
	CONSTRAINT FK_ingredientId FOREIGN KEY (ingredient_id) REFERENCES ingredients(id) ON DELETE CASCADE,
	CONSTRAINT FK_stepId FOREIGN KEY (step_id) REFERENCES steps(id) ON DELETE CASCADE
);
CREATE INDEX pkid_components ON components (id);
CREATE INDEX fkstep_id_component ON components (step_id);

CREATE TABLE notes (
	id SERIAL UNIQUE, 
	text text, 
	date date, 
	recipe_id integer,
	step_id integer,
	CONSTRAINT PK_notes PRIMARY KEY(id) ON DELETE CASCADE
	CONSTRAINT FK_recipeId_notes FOREIGN KEY (recipe_id) REFERENCES recipes (id) ON DELETE CASCADE,
	CONSTRAINT FK_stepId_notes FOREIGN KEY (step_id) REFERENCES steps (id) ON DELETE CASCADE
);
CREATE INDEX pkid_notes ON notes (id);
CREATE INDEX fkrecipe_id_notes ON notes (recipe_id);
CREATE INDEX fkstep_id_notes ON notes (step_id);