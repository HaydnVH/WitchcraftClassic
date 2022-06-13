The scene json format is an easy way to define the contents of a scene in a human-readable manner.  The format is designed to be easy for a human to read and modify, rather than being in the format which is most convenient for loading, and is expected to be converted to an optimized binary file before being shipped.

A scene's ID is equivalent to its filepath relative to the "scenes/" directory, minus the file extension.  If a scene is located at "data_files/scenes/my_example_scene.scene.json", the scene's ID will be "my_example_scene".

"skybox": "textures/my_skybox/sky"

This line, typically at the top of the scene json structure, indicates that the skybox for this scene is located at "data_files/textures/my_skybox/sky.json".  The file path is relative to the main "data_files/" directory.  SUBJECT TO CHANGE.

"entrances": { ... }

Defines how this scene might be entered.  When the scene is loaded, the player will be placed at the location indicated by the entrance.  If no entrance or an invalid entrance is specified, the position of the player and camera will be undefined (but probably placed at the scene's local origin).

"entities": { ...

This object stores the records for every entity in the scene.  The key for each entity is its numeric ID; the ID must be unique, and should be greater than zero.  The value, in this case, is another object storing the components of the entity.

"name": "ExampleEntityName",

Stores the name of the entity, which is useful for debugging and can be used to search for the entity.

"transform": { ... }

Stores the initial position, rotation, and scale of the entity.  The transform component may contain up to 3 key-value pairs. "position", with an array of 3 numbers, initializes the entity's position; if this is not present, the default is [0, 0, 0]. "rotation", with an array of 3 numbers, initializes the entity's orientation by interpreting the 3 values as euler angles; if this is not present, the default is [0, 0, 0].  "scale", with an array of 3 numbers, initializes the entity's scale; if this is not present, the default is [1, 1, 1].

 "model": "ExampleEntityModel.wcm.xml"
 
 Stores the path of the model which will be drawn at the entity's position.  This path is relative to "data_files/models/".
 
 "animator": { ... }
 
 Indicates that the entity's model should be animated.  Optionally, may contain an "import" array, containing a list of model files whose animations should be imported into this entity's model.
 
 "collider": { ... }
 
 Contains instructions on how to create a collider for this entity.  If this is the string "load_from_model", the definition of the collider will be loaded from the entity's model file.  If this is the string "create_from_model", a static triangle mesh will be created from the model's geometry.  Otherwise, a shape and its dimensions will be indicated.
 
 "rigidbody": { ... }
 
 Contains instructions on how to create a rigidbody for this entity.  "mass" is how much mass the object should have; 0 indicates that it is static and will not be affected by the physics simulation.  "kinematic" is a true/false value indicating whether or not scripts are allowed to modify the entity's transform and have those changes reflected by the physics simulation.  "trigger" 
 
 "scripts": { ... }
 
 Contains a list of scripts which should be attached to this entity.  Each script name pairs with an object which is converted to a lua table and passed to that script's "on_init()" function.