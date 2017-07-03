/** \file
 * Topologically sort vectors for faster laser cutting or plotting.
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct _vector vector_t;
struct _vector
{
	vector_t * next;
	vector_t ** prev;
	double x1;
	double y1;
	double x2;
	double y2;
};


typedef struct
{
	vector_t * vectors;
} vectors_t;

// Red/Green/Blue
#define VECTOR_PASSES 3


static void
vector_stats(
	vector_t * v
)
{
	double lx = 0;
	double ly = 0;
	double cut_len_sum = 0;
	int cuts = 0;

	double transit_len_sum = 0;
	int transits = 0;

	while (v)
	{
		double t_dx = lx - v->x1;
		double t_dy = ly - v->y1;

		double transit_len = sqrt(t_dx * t_dx + t_dy * t_dy);
		if (transit_len != 0)
		{
			transits++;
			transit_len_sum += transit_len;
		}

		double c_dx = v->x1 - v->x2;
		double c_dy = v->y1 - v->y2;

		double cut_len = sqrt(c_dx*c_dx + c_dy*c_dy);
		if (cut_len != 0)
		{
			cuts++;
			cut_len_sum += cut_len;
		}

		// Advance the point
		lx = v->x2;
		ly = v->y2;
		v = v->next;
	}

	fprintf(stderr, "Cuts: %u len %.0f\n", cuts, cut_len_sum);
	fprintf(stderr, "Move: %u len %.0f\n", transits, transit_len_sum);
}


static void
vector_create(
	vectors_t * const vectors,
	int x1,
	int y1,
	int x2,
	int y2
)
{
	// Find the end of the list and, if vector optimization is
	// turned on, check for duplicates
	vector_t ** iter = &vectors->vectors;
	while (*iter)
	{
		vector_t * const p = *iter;

		if (p->x1 == x1 && p->y1 == y1
		&&  p->x2 == x2 && p->y2 == y2)
			return;

		if (p->x1 == x2 && p->y1 == y2
		&&  p->x2 == x1 && p->y2 == y1)
			return;

		if (x1 == x2
		&&  y1 == y2)
			return;

		iter = &p->next;
	}

	vector_t * const v = calloc(1, sizeof(*v));
	if (!v)
		return;

	v->x1 = x1;
	v->y1 = y1;
	v->x2 = x2;
	v->y2 = y2;

	// Append it to the now known end of the list
	v->next = NULL;
	v->prev = iter;
	*iter = v;
}



/**
 * Generate a list of vectors.
 *
 * The vector format is:
 * Pr,g,b -- Color of the vector
 * Mx,y -- Move (start a line at x,y)
 * Lx,y -- Line to x,y from the current position
 * C -- Closing line segment to the starting position
 * X -- end of file
 *
 * Multi segment vectors are split into individual vectors, which are
 * then passed into the topological sort routine.
 *
 * Exact duplictes will be deleted to try to avoid double hits..
 */
static vectors_t *
vectors_parse(
	FILE * const vector_file
)
{
	vectors_t * const vectors = calloc(VECTOR_PASSES, sizeof(*vectors));
	double mx = 0, my = 0;
	double lx = 0, ly = 0;
	int pass = 0;
	int count = 0;

	char buf[256];

	while (fgets(buf, sizeof(buf), vector_file))
	{
		//fprintf(stderr, "read '%s'\n", buf);
		const char cmd = buf[0];
		double x, y;

		switch (cmd)
		{
		case 'P':
		{
			// note that they will be in bgr order in the file
			int r, g, b;
			sscanf(buf+1, "%d,%d,%d", &b, &g, &r);
			if (r == 0 && g != 0 && b == 0)
			{
				pass = 0;
			} else
			if (r != 0 && g == 0 && b == 0)
			{
				pass = 1;
			} else
			if (r == 0 && g == 0 && b != 0)
			{
				pass = 2;
			} else {
				fprintf(stderr, "non-red/green/blue vector? %d,%d,%d\n", r, g, b);
				exit(-1);
			}
			break;
		}
		case 'M':
			// Start a new line.
			// This also implicitly sets the
			// current laser position
			sscanf(buf+1, "%lf,%lf", &mx, &my);
			lx = mx;
			ly = my;
			break;
		case 'L':
			// Add a line segment from the current
			// point to the new point, and update
			// the current point to the new point.
			sscanf(buf+1, "%lf,%lf", &x, &y);
			vector_create(&vectors[pass], lx, ly, x, y);
			count++;
			lx = x;
			ly = y;
			break;
		case 'C':
			// Closing segment from the current point
			// back to the starting point
			vector_create(&vectors[pass], lx, ly, mx, my);
			lx = mx;
			lx = my;
			break;
		case 'X':
			goto done;
		default:
			fprintf(stderr, "Unknown command '%c'", cmd);
			return NULL;
		}
	}

done:
	fprintf(stderr, "read %u segments\n", count);
	for (int i = 0 ; i < VECTOR_PASSES ; i++)
	{
		vector_stats(vectors[i].vectors);
	}

	fprintf(stderr, "---\n");

	return vectors;
}


/** Find the closest vector to a given point and remove it from the list.
 *
 * This might reverse a vector if it is closest to draw it in reverse
 * order.
 */
static vector_t *
vector_find_closest(
	vector_t * v,
	const int cx,
	const int cy
)
{
	double best_dist = 1e9;
	vector_t * best = NULL;
	int do_reverse = 0;

	while (v)
	{
		double dx1 = cx - v->x1;
		double dy1 = cy - v->y1;
		double dist1 = dx1*dx1 + dy1*dy1;

		if (dist1 < best_dist)
		{
			best = v;
			best_dist = dist1;
			do_reverse = 0;
		}

		double dx2 = cx - v->x2;
		double dy2 = cy - v->y2;
		double dist2 = dx2*dx2 + dy2*dy2;
		if (dist2 < best_dist)
		{
			best = v;
			best_dist = dist2;
			do_reverse = 1;
		}

		v = v->next;
	}

	if (!best)
		return NULL;

	// Remove it from the list
	*best->prev = best->next;
	if (best->next)
		best->next->prev = best->prev;

	// If reversing is required, flip the x1/x2 and y1/y2
	if (do_reverse)
	{
		double x1 = best->x1;
		double y1 = best->y1;
		best->x1 = best->x2;
		best->y1 = best->y2;
		best->x2 = x1;
		best->y2 = y1;
	}

	best->next = NULL;
	best->prev = NULL;

	return best;
}


/**
 * Optimize the cut order to minimize transit time.
 *
 * Simplistic greedy algorithm: look for the closest vector that starts
 * or ends at the same point as the current point. 
 *
 * This does not split vectors.
 */
static int
vector_optimize(
	vectors_t * const vectors
)
{
	static int cx = 0;
	static int cy = 0;

	vector_t * vs = NULL;
	vector_t * vs_tail = NULL;

	while (vectors->vectors)
	{
		vector_t * v = vector_find_closest(vectors->vectors, cx, cy);
		if (!v)
		{
			fprintf(stderr, "nothing close?\n");
			abort();
		}

		if (!vs)
		{
			// Nothing on the list yet
			vs = vs_tail = v;
		} else {
			// Add it to the tail of the list
			v->next = NULL;
			v->prev = &vs_tail->next;
			vs_tail->next = v;
			vs_tail = v;
		}
		
		// Move the current point to the end of the line segment
		cx = v->x2;
		cy = v->y2;
	}

	vector_stats(vs);

	// Now replace the list in the vectors object with this new one
	vectors->vectors = vs;
	if (vs)
		vs->prev = &vectors->vectors;

	return 0;
}


static void
output_vector(
	FILE * const pjl_file,
	const vector_t * v
)
{
	double lx = 0;
	double ly = 0;

	while (v)
	{
		if (v->x1 != lx || v->y1 != ly)
		{
			// Stop the laser; we need to transit
			// and then start the laser as we go to
			// the next point.  Note initial ";"
			fprintf(pjl_file, ";PU%.3f,%.3f;PD%.3f,%.3f",
				v->y1,
				v->x1,
				v->y2,
				v->x2
			);
		} else {
			// This is the continuation of a line, so
			// just add additional points
			fprintf(pjl_file, ",%.3f,%.3f",
				v->y2,
				v->x2
			);
		}

		// Changing power on the fly is not supported for now
		// \todo: Check v->power and adjust ZS, XR, etc

		// Move to the next vector, updating our current point
		lx = v->x2;
		ly = v->y2;
		v = v->next;
	}
}

				
static void
generate_vectors(
	FILE * const vector_file,
	FILE * const pjl_file
)
{
	vectors_t * const vectors = vectors_parse(vector_file);

	for (int i = 0 ; i < VECTOR_PASSES ; i++)
	{
		vector_optimize(&vectors[i]);

		const vector_t * v = vectors[i].vectors;

		output_vector(pjl_file, v);
	}
}


int main(void)
{
	generate_vectors(stdin, stdout);
	return 0;
}
