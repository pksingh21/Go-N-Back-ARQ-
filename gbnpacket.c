/* gbnpacket.c - defines the go-back-n packet structure
 * by Prateek Kumar Singh <20cs01003@iitbbs.ac.in>
 */
struct gbnpacket
{
  int type;
  int seq_no;
  int length;
  char data[512];
};
