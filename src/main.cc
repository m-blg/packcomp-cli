#include "argp.h"

#include "packcomp/packcomp.h"
#include "string.h"

#ifdef VERSION
const char *argp_program_version = "packcomp " VERSION;
#endif

static char doc[] = 
"packcomp fetches two branches, finds unique packages in both of them and also packages "
"version of which in the first branch is greater than version of the corresponding packages "
"in the second branch. This operation is performed for every specified package architecture "
"(-a) or for every common architecture if none specified.";

static char args_doc[] = "[BRANCH1] [BRANCH2]";

static struct argp_option options[] = {
    {"verbose", 'v', 0, 0, "Produce verbose output"},
    {"archs", 'a', "ARCH1,ARCH2,...", 0, "Specify architecture"},
    {"output", 'o', "FILE", 0, "Output to FILE instead of standard output"},
    {0}};

struct InputArgs {
    const char *arch = "";
    const char *out_file = "";
    const char *branches[2];
};

static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  InputArgs *input_args = (InputArgs*) state->input;

    switch (key)
    {
    case 'v':
        g_packcomp_verbose = 1;
        break;
    case 'a':
        input_args->arch = arg;
        break;
    case 'o':
        input_args->out_file = arg;
        break;
    case ARGP_KEY_ARG:
        if (state->arg_num >= 2)
        {
            argp_usage(state);
        }
        input_args->branches[state->arg_num] = arg;
        break;
    case ARGP_KEY_END:
        if (state->arg_num < 2)
        {
            argp_usage(state);
        }
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

int main(int argc, char *argv[])
{
    InputArgs input_args;
    g_packcomp_verbose = 0;

    argp_parse(&argp, argc, argv, 0, 0, &input_args);
    // auto jsons = package_compare_all_archs(input_args.branches[0], input_args.branches[1]);

    auto branch1 = input_args.branches[0];
    auto branch2 = input_args.branches[1];

    const char* *common_archs; size_t common_archs_len;
    if (!get_common_archs(branch1, branch2, &common_archs, &common_archs_len)) {
        fprintf(stderr, "fetching archs failed");
        return EXIT_FAILURE;
    }
    if (common_archs_len == 0) {
        printf("no common archs\n");
        return EXIT_SUCCESS;
    }

    // parse archs
    char* s = NULL; // size_t s_len;
    const char* *archs = NULL; size_t archs_len=0;
    if (strcmp(input_args.arch, "") != 0) {
        size_t s_len = strlen(input_args.arch);
        char* s = (char*)malloc((s_len+1) * sizeof(char));
        strncpy(s, input_args.arch, s_len+1);
        char* context;

        archs = (const char**)malloc(common_archs_len * sizeof(const char*));
        for (;;s = NULL) {
            char* token = strtok_r(s, ",", &context);
            if (token == NULL)
                break;
            
            bool b = false;
            for (size_t i = 0; i < common_archs_len; i++) {
                if (strcmp(token, common_archs[i]) == 0) {
                    archs[archs_len] = token;
                    archs_len++;
                    b = true;
                    break;
                } 
            }

            if (!b) {
                fprintf(stderr, "there is no such arch: %s\n", s);
            }

        }
    } else {
        archs = common_archs;
        archs_len = common_archs_len;
    }

    if (!archs_len) {
        printf("there is nothing to do\n");
        return EXIT_SUCCESS;
    }

    if (g_packcomp_verbose) {
        printf("archs: ");
        for (size_t i = 0; i < archs_len; i++) {
            printf(archs[i]);
            printf(", ");
        }
        printf("\n");
    }

    size_t jsons_len;
    auto jsons = package_compare(branch1, branch2, archs, archs_len, &jsons_len, compare_sorted);
    if (!jsons_len) {
        fprintf(stderr, "comparison failed\n");
        return EXIT_FAILURE;
    }


    FILE* ofile = stdout;
    if (strcmp(input_args.out_file, "") != 0) {
        ofile = fopen(input_args.out_file, "w");
    }

    for (size_t i=0; i < jsons_len; i++) {
        fprintf(ofile, "%s\n", json_object_to_json_string(jsons[i]));
    }

    if (strcmp(input_args.out_file, "") != 0) {
        fclose(ofile);
    }

    for (size_t i=0; i < jsons_len; i++) {
        json_object_put(jsons[i]);
    }
    
    if (s != NULL)
        free(s); 
    if (archs != NULL)
        free(archs);
}
