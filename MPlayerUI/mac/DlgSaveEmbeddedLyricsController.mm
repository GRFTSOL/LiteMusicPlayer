//
//  DlgSaveEmbeddedLyricsController.m
//  MiniLyricsMac
//
//  Created by Xiao Hongyong on 6/7/13.
//
//

#import <Cocoa/Cocoa.h>
#import "../../Window/mac/Window.h"
#import "../../Window/mac/WindowHandleHolder.h"
#import "DlgSaveEmbeddedLyricsController.h"
#import "MPlayerApp.h"

#import "DlgSaveEmbeddedLyrics.h"


string m_strMediaUrl;
string m_bufLyrics;

uint32_t m_nDefaultEmbededLST;
bool m_bHasEmbeddedLyrAlready;

@implementation DlgSaveEmbeddedLyricsController


- (void) doModal:(CDlgSaveEmbeddedLyrics *)dialog {
    mDialg = dialog;
    [NSApp beginSheet: [self window]
        modalForWindow: CMPlayerAppBase::getMainWnd()->getHandleHolder()->window// nil//(NSWindow*)dialog->GetHandle()
        modalDelegate: nil
        didEndSelector: nil
        contextInfo: nil];

    [NSApp runModalForWindow: [self window]];
    // Dialog is up here.
    [NSApp endSheet: [self window]];
    [[self window] orderOut: self];
}

- (IBAction)buttonOK:(id)sender {
    [NSApp stopModal];
    mDialg->onOK();
}

- (IBAction)buttonCancel:(id)sender {
    [NSApp stopModal];
}

- (id)initWithWindow:(NSWindow *)window {
    self = [super initWithWindow:window];
    if (self) {
        // Initialization code here.

        _tableDesc = [NSMutableArray new];
        _tableName = [NSMutableArray new];
        _tableCheck = [NSMutableArray new];
    }

    return self;
}

- (void)addItem:(const char *)text name:(const char *)name isChecked:(bool)isChecked {
    [_tableDesc addObject:[NSString stringWithUTF8String:text]];
    [_tableName addObject:[NSString stringWithUTF8String:name]];
    [_tableCheck addObject:[NSNumber numberWithBool: isChecked]];
}

- (int)getItemCount {
    return (int)[_tableDesc count];
}

- (const char*)getNameAtIndex:(long)index {
    return [[_tableName objectAtIndex:index] UTF8String];
}

- (bool)getCheckAtIndex:(long)index {
    return [[_tableCheck objectAtIndex:index] boolValue];
}

- (void)windowDidLoad {
    [super windowDidLoad];

    NSTableColumn *column = [[[self tableView] tableColumns] objectAtIndex:0];
    [[column headerCell] setStringValue: [NSString stringWithUTF8String: _TLT("Embedded Lyrics Type")]];
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView {
    return [_tableName count];
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
    if ([[tableColumn identifier] isEqualTo:@"check"]) {
        return [_tableCheck objectAtIndex:row];
    } else if ([[tableColumn identifier] isEqualTo:@"desc"]) {
        return [_tableDesc objectAtIndex:row];
    } else {
        // Return the same value for any table column -- it is just a string
    }
    return [_tableName objectAtIndex:row];
}

- (void)tableView:(NSTableView *)tableView setObjectValue:(id)value forTableColumn:(NSTableColumn *)column row:(NSInteger)row {
    [_tableCheck replaceObjectAtIndex:row withObject:value];
    [_tableView reloadData];
}

@end
