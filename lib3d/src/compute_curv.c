/* $Id: compute_curv.c,v 1.1 2003/05/09 11:45:37 aspert Exp $ */
#include <3dutils.h>
#include <model_in.h>
#include <ring.h>
#include <curvature.h>


int main(int argc, char **argv) {
  struct model *raw_model=NULL;
  struct info_vertex *info;
  struct ring_info *ring;
  int i;
  char *filename;
  double maxkm=-FLT_MAX, maxkg=-FLT_MAX;
  double minkm=FLT_MAX, minkg=FLT_MAX;



  if (argc != 2) {
    fprintf(stderr, "Usage: compute_curv or_file.raw\n");
    exit(-1);
  }
  filename = argv[1];

  
  i = read_fmodel(&raw_model, filename, MESH_FF_AUTO, 1);
  if (raw_model == NULL || i<0 ) {
    fprintf(stderr, "Unable to read model, error code %d\n", i);
    exit(-1);
  }



  printf("Computing face normals...\n");
  info = (struct info_vertex*)
    malloc(raw_model->num_vert*sizeof(struct info_vertex));
  ring = (struct ring_info*)
    malloc(raw_model->num_vert*sizeof(struct ring_info));
  
  raw_model->face_normals = compute_face_normals(raw_model, ring);

  printf("Computing vertex normals...\n");
  raw_model->area = (float*)malloc(raw_model->num_faces*sizeof(float));

  compute_vertex_normal(raw_model, ring, raw_model->face_normals); 


  
  printf("Computing curvature of model.... ");fflush(stdout);
  compute_curvature_with_rings(raw_model, info, ring);
  printf("done\n");

  for (i=0; i<raw_model->num_vert; i++) {

    if (info[i].gauss_curv > maxkg)
      maxkg = info[i].gauss_curv;
    if (info[i].gauss_curv < minkg)
      minkg = info[i].gauss_curv;

    if (info[i].mean_curv > maxkm)
      maxkm = info[i].mean_curv;
    if (info[i].mean_curv < minkm)
      minkm = info[i].mean_curv;
  }

  printf("minkm = %f\tmaxkm = %f\n", minkm, maxkm);
  printf("minkg = %f\tmaxkg = %f\n", minkg, maxkg);
  for (i=0; i<raw_model->num_vert; i++) {
    free(ring[i].ord_face);
    free(ring[i].ord_vert);
  }

  free(info);

  free(ring);

  __free_raw_model(raw_model);

  return 0;
}
