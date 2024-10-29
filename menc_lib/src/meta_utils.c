Public void Meta_Mark_As_String(String* out, String mark, b8 isBegin, b8 isPreprocessed) {
    String  postFix = ( isBegin ) ? S("Begin") : S("End");

    Str_From_Fmt(out, ( isPreprocessed ) ? "int %s_%s;\n" : "Meta_Mark(%s,%s)\n", mark.E, postFix.E);
}
