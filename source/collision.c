inline internal b8 CheckOverlap(AABB a, AABB b)
{
	if (a.max.x < b.min.x || a.min.x > b.max.x) return false;
	if (a.max.y < b.min.y || a.min.y > b.max.y) return false;
	
	return true;
}