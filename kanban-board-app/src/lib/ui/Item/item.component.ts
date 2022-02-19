import { Component, Input } from '@angular/core';

@Component({
  selector: 'item',   // Zugriffstag
  templateUrl: './item.component.html',
  styleUrls: ['./item.component.scss'],
})
export class ItemComponent {
  @Input()
  title = 'item';

  @Input()
  id = 'item';
}
