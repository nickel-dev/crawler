Entity* NewEntity(State* state)
{
	Entity* result = state->entities + state->entitiesSize;
	++state->entitiesSize;
	
	result->scale = V2Scalar(1.0f);
	result->texture = state->errorTexture;
	
	result->health = 1;
	result->holdItem = true;
	
	result->speed = 1;
	result->destroyable = true;
	
	return result;
}

Entity* NewEnemy(State* state)
{
	Entity* e = NewEntity(state);
	state->enemies[state->enemiesSize] = e;
	++state->enemiesSize;
	e->useAi = true;
	return e;
}

Item* NewItem(State* state)
{
	Item* result = state->items + state->itemsSize;
	++state->itemsSize;
	
	result->damage = 1.0f;
	result->cooldownTime = 0.5f;
	
	return result;
}

global v4 potionEffectColors[] =
{
	{ 1.0f, 0.0f, 0.0f, 1.0f },
	{ 1.0f, 0.5f, 0.0f, 1.0f },
	{ 0.0f, 0.8f, 0.0f, 1.0f },
};

Item* NewPotion(State* state, u32 outlineTexture, u32 fillTexture)
{
	Item* result = state->items + state->itemsSize;
	++state->itemsSize;
	
	result->damage = 0;
	result->isPotion = true;
	result->effect = rand() % (POTION_EFFECT_SPEED + 1);
	result->fillTint = potionEffectColors[result->effect];
	result->fill = fillTexture;
	result->texture = outlineTexture;
	
	return result;
}

AABB EntityAABB(Entity* e)
{
	AABB result;
	result.min = V2AddV2(e->pos, V2Scalar(0.05f));
	result.max = V2SubV2(V2AddV2(e->pos, e->scale), V2Scalar(0.05f));
	return result;
}

v2 SmoothFollow(v2 follower, v2 target, f32 maxDistance, f32 lag)
{
    // Calculate the difference between the target and follower
    float dx = target.x - follower.x;
    float dy = target.y - follower.y;
    
    // Calculate the distance between the target and follower
    float distance = sqrt(dx * dx + dy * dy);
    
    // If the distance is greater than the max distance, move the follower
    if (distance > maxDistance) {
        // Calculate the direction vector
        float directionX = dx / distance;
        float directionY = dy / distance;
        
        // Move the follower towards the target with a slight lag
        follower.x += directionX * (distance - maxDistance) * lag;
        follower.y += directionY * (distance - maxDistance) * lag;
    } else {
        // Center the follower with a lag effect
        follower.x += dx * lag;
        follower.y += dy * lag;
    }
    
    return follower;
}

//~ NOTE(nickel): Player functions
#define PLAYER_SPEED 4.0f

void MovePlayer(Entity* e, v2 dir)
{
	dir = V2Normalize(dir);
	e->vel  = V2(PLAYER_SPEED * dir.x * e->speed, PLAYER_SPEED * dir.y * e->speed);
}

v2 MoveTowards(v2 pos, v2 dest)
{
	v2 result = V2(dest.x - pos.x, dest.y - pos.y);
	
	f32 hyp = sqrt(result.x * result.x + result.y * result.y);
	result = V2DivScalar(result, hyp);
	
	return result;
}

f32 RotateTowards(v2 pos, v2 lookat)
{
    f32 result;
    i32 deltaX;
    i32 deltaY;
    deltaX = pos.x - lookat.x;
    deltaY = pos.y - lookat.y;
    result = (atan2(deltaX, deltaY) * 180.0f) / PI;
    return result;
}

void WobbleAnimation(Entity* e, f64 dt)
{
	v2 vel = e->vel;
	if (e->stunned)
		vel = V2Scalar(0.0f);
	
	if (vel.x != 0.0f || vel.y != 0.0f)
	{
		e->wobbleFrame += dt * 800;
		e->angle = 7.5f * FastSin((i32)e->wobbleFrame);
	}
	else if (e->angle != 0.0f)
	{
		e->wobbleFrame += dt * 800;
		e->angle = 7.5f * FastSin((i32)e->wobbleFrame);
		
		if (e->angle > -0.25f && e->angle < 0.25f)
			e->angle = 0.0f;
	}
	else
	{
		e->wobbleFrame = 0.0;
		e->angle = 0.0f;
	}
}

v2 PositionTowards(f32 distance, v2 pos, v2 dest)
{
	v2 result = V2MulV2(MoveTowards(pos, dest), V2(distance, -distance));
	return V2AddV2(result, pos);
}