# houdini-sop-voxel-center-points

[Houdini](http://www.sidefx.com/index.php) SOP node which generates a center point for each voxel in a volume.

![houdini-sop-voxel-center-points screenshot](http://i.imgur.com/4oS43vh.png)

## Binaries, Houdini 15
* Mac OS X - Todo.
* Windows - Todo.

## Building

* Tested on OS X 10.11 and Houdini 15.
  * You would have to patch CMake file to get this building on Windows or Linux.
* Define HOUDINI_VERSION env variable to be the version of Houdini 15 you wish to build against (for example "15.0.313").
* Alternatively, you can have HFS env variable defined (set when you source houdini_setup).
* Generate build files from CMake for your favorite build system.
* Build the SOP Houdini dso (SOP_VoxelCenterPoints.dylib or SOP_VoxelCenterPoints.dll).
* Place the dso in the appropriate Houdini dso folder.
  * On OS X this would be /Users/your_username/Library/Preferences/houdini/15.0/dso/

## Usage

* Place Voxel Center Points SOP in your SOP network.
* Use any volume as an input. For each non-zero value voxel a point will be generated.
* Parameters are:
    * Maintain Volume Transform - input volume transform will be kept when generating points.)
    * Create Point Value Attribute - a point attribute (named after volume) will be created and value will be set to the voxel value.
    * Override the Attribute Name - specify a custom name for point values.

## License

* Copyright Mykola Konyk, 2016
* Distributed under the [MS-RL License.](http://opensource.org/licenses/MS-RL)
* **To further explain the license:**
  * **You cannot re-license any files in this project.**
  * **That is, they must remain under the [MS-RL license.](http://opensource.org/licenses/MS-RL)**
  * **Any other files you add to this project can be under any license you want.**
  * **You cannot use any of this code in a GPL project.**
  * Otherwise you are free to do pretty much anything you want with this code.
