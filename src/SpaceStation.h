#ifndef _SPACESTATION_H
#define _SPACESTATION_H

#include "libs.h"
#include "StaticRigidBody.h"

class SpaceStation: public StaticRigidBody {
public:
	SpaceStation();
	virtual ~SpaceStation();
	virtual bool OnCollision(Body *b);
	virtual Object::Type GetType() { return Object::SPACESTATION; }
	virtual void Render(const Frame *camFrame);
protected:
};

#endif /* _SPACESTATION_H */