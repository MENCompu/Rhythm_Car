#ifndef RC_ENTITY_H
#define RC_ENTITY_H

typedef struct {
    Transform transform;

    Asset_Handle model;
} Entity;
Array(Array_Entity, Entity);

#endif // RC_ENTITY_H
