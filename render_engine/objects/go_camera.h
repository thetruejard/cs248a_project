#pragma once
#include "objects/gameobject.h"


/*
* A simple GameObject containing only a static mesh.
*/
class GO_Camera : public GameObject {
public:

	GO_Camera(GameObjectID id, RenderEngine* engine);
	virtual ~GO_Camera() override = default;
	virtual std::string getTypeName() override;

	/*
	* Recursively sets the dirty flag for all children.
	* We override this so we can also set the dirty flag for the view matrix.
	*/
	virtual void setModelMatrixDirty() override;


	/*
	* Camera properties.
	*/

	/*
	* Returns the view matrix, which is the inverse of the camera's model matrix.
	*/
	const glm::mat4& getViewMatrix();
	
	/*
	* Returns the camera's projection matrix.
	*/
	const glm::mat4& getProjectionMatrix();

	/*
	* Sets the projection matrix to an orthographic matrix.
	* yradius is the distance from the center to the top OR bottom.
	* The aspect can be changed later with setAspect().
	* If the camera is scaled, all planes are scaled too.
	*/
	void setOrthographic(float yradius, float aspect, float near, float far);

	/*
	* Sets the projection matrix to an orthographic matrix.
	* With this version, the aspect is NOT corrected with setAspect().
	* If the camera is scaled, all planes are scaled too.
	*/
	void setOrthographic(
		float left, float right, float top, float bottom, float near, float far
	);

	/*
	* Sets the projection matrix to a perspective matrix.
	* fovy is the vertical field of view in radians.
	* The aspect can be changed later with setAspect().
	* If the camera is scaled, the near and far planes are scaled too.
	*/
	void setPerspective(float fovy, float aspect, float near, float far);

	/*
	* Sets the projection matrix to a custom projection.
	* The aspect cannot be changed using setAspect().
	*/
	void setCustomProjection(const glm::mat4& proj);

	/*
	* Corrects the current projection matrix to fit the given aspect ratio, if possible.
	* If the current projection type cannot have aspect corrected, does nothing.
	*/
	void setAspect(float aspect);


// private:			// TEMP: For simplicity, just make everything public

	enum class ProjectionType {
		Ortho_yradius,			// Orthographic defined by a yradius and aspect.
		Ortho_planes,			// Orthographic defined by 6 clipping planes.
		Perspective,			// Perspective defined by an FOV and aspect.
		Custom,					// Custom defined by a user-provided matrix.
	};

	ProjectionType projectionType = ProjectionType::Custom;

	/*
	* Projection parameters are only stored if they might be relevant later.
	* For now, this only includes recomputation of matrices when the aspect changes.
	*/
	union {

		struct {
			float yradius;
			float near;
			float far;
		} ortho_yradius;
		
		struct {
			float fovy;
			float near;
			float far;
			float aspect;
		} perspective;
	
	} projectionParams;

	glm::mat4 projectionMatrix = glm::mat4(1.0f);

	glm::mat4 viewMatrix = glm::mat4(1.0f);
	bool viewMatrixDirty = true;

};